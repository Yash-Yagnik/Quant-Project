"""
Queue position simulation for HFT backtesting.

Models where our order sits in the LOB; we only get filled when volume ahead of us
is traded through. Uses price-time priority.
"""

from dataclasses import dataclass, field
from typing import List, Optional
import numpy as np


@dataclass
class QueueOrder:
    """Order in the queue at a price level."""
    order_id: int
    price: float
    qty: float
    side: str  # "bid" or "ask"
    queue_position: int  # volume ahead of this order at same price
    cumulative_ahead: float  # total volume ahead in book (same price, time priority)


@dataclass
class PriceLevel:
    """Price level with queue (list of orders in time order)."""
    price: float
    side: str
    orders: List[QueueOrder] = field(default_factory=list)

    @property
    def total_volume(self) -> float:
        return sum(o.qty for o in self.orders)

    def volume_ahead_of(self, order_id: int) -> float:
        """Volume ahead of given order at this level (time priority)."""
        v = 0.0
        for o in self.orders:
            if o.order_id == order_id:
                return v
            v += o.qty
        return v


class QueueSimLOB:
    """
    Limit order book that tracks queue position.
    Fills only occur when traded volume consumes the queue ahead of our order.
    """

    def __init__(self, tick_size: float = 0.01):
        self.tick_size = tick_size
        self.bids: dict[float, PriceLevel] = {}  # price -> level
        self.asks: dict[float, PriceLevel] = {}
        self._order_to_level: dict[int, tuple[float, str]] = {}  # order_id -> (price, side)

    def add_order(self, order_id: int, price: float, qty: float, side: str) -> QueueOrder:
        levels = self.bids if side == "bid" else self.asks
        price = round(price / self.tick_size) * self.tick_size
        if price not in levels:
            levels[price] = PriceLevel(price=price, side=side)
        level = levels[price]
        cumulative_ahead = level.total_volume
        order = QueueOrder(
            order_id=order_id,
            price=price,
            qty=qty,
            side=side,
            queue_position=len(level.orders),
            cumulative_ahead=cumulative_ahead,
        )
        level.orders.append(order)
        self._order_to_level[order_id] = (price, side)
        return order

    def cancel_order(self, order_id: int) -> bool:
        if order_id not in self._order_to_level:
            return False
        price, side = self._order_to_level[order_id]
        levels = self.bids if side == "bid" else self.asks
        if price not in levels:
            return False
        level = levels[price]
        level.orders = [o for o in level.orders if o.order_id != order_id]
        if not level.orders:
            del levels[price]
        del self._order_to_level[order_id]
        return True

    def trade_at_level(self, price: float, side: str, traded_qty: float) -> List[tuple[int, float]]:
        """
        Simulate trade consuming from the front of the queue at this level.
        Returns list of (order_id, fill_qty) for orders that get filled.
        """
        levels = self.bids if side == "bid" else self.asks
        if price not in levels:
            return []
        level = levels[price]
        fills: List[tuple[int, float]] = []
        remaining = traded_qty
        while remaining > 0 and level.orders:
            order = level.orders[0]
            fill_qty = min(order.qty, remaining)
            fills.append((order.order_id, fill_qty))
            remaining -= fill_qty
            order.qty -= fill_qty
            if order.qty <= 0:
                level.orders.pop(0)
                self._order_to_level.pop(order.order_id, None)
            else:
                break
        if not level.orders:
            del levels[price]
        return fills

    def best_bid(self) -> Optional[float]:
        return max(self.bids.keys()) if self.bids else None

    def best_ask(self) -> Optional[float]:
        return min(self.asks.keys()) if self.asks else None

    def mid(self) -> Optional[float]:
        b, a = self.best_bid(), self.best_ask()
        if b is None or a is None:
            return None
        return (b + a) / 2
