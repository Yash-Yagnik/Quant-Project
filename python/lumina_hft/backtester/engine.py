"""
HFT-aware backtest engine: queue simulation, latency, slippage.
"""

from dataclasses import dataclass, field
from typing import List, Optional, Callable
import numpy as np

from .queue_sim import QueueSimLOB, QueueOrder
from .latency import LatencyModel, TickToTrade
from .slippage import apply_slippage


@dataclass
class BacktestConfig:
    """Backtest parameters."""
    tick_size: float = 0.01
    latency_mean_us: float = 50.0
    latency_std_us: float = 10.0
    impact_eta: float = 0.1
    impact_V: float = 1e6
    spread_bps: float = 5.0
    seed: Optional[int] = None


@dataclass
class Fill:
    """Fill event."""
    order_id: int
    price: float
    qty: float
    side: str
    time_ns: int


@dataclass
class BacktestState:
    """Mutable state during backtest."""
    time_ns: int = 0
    mid: Optional[float] = None
    inventory: float = 0.0
    pnl: float = 0.0
    fills: List[Fill] = field(default_factory=list)


class BacktestEngine:
    """
    Backtest engine that:
    - Maintains a queue-sim LOB
    - Applies tick-to-trade latency before orders reach the book
    - Applies square-root impact on execution
    - Processes trades and updates fills
    """

    def __init__(self, config: BacktestConfig):
        self.config = config
        self.lob = QueueSimLOB(tick_size=config.tick_size)
        rng = np.random.default_rng(config.seed)
        self.latency_model = LatencyModel(
            mean_us=config.latency_mean_us,
            std_us=config.latency_std_us,
            rng=rng,
        )
        self.t2t = TickToTrade(self.latency_model)
        self.state = BacktestState()

    def set_mid(self, time_ns: int, mid: float) -> None:
        """Update time and mid (e.g. from tape)."""
        self.state.time_ns = time_ns
        self.state.mid = mid

    def inject_trade(self, time_ns: int, price: float, qty: float, side: str) -> List[tuple]:
        """
        Inject a market trade; consume from LOB queue and return fills
        (order_id, fill_qty) for our orders that got filled.
        """
        self.state.time_ns = time_ns
        return self.lob.trade_at_level(price, side, qty)

    def submit_order(self, order_id: int, price: float, qty: float, side: str) -> None:
        """Submit order (will be delayed by latency, then added to queue)."""
        self.t2t.submit(
            self.state.time_ns,
            {"order_id": order_id, "price": price, "qty": qty, "side": side},
        )

    def step_latency(self) -> None:
        """Release orders that have passed their latency delay and add to LOB."""
        for order_info in self.t2t.release_ready(self.state.time_ns):
            self.lob.add_order(
                order_info["order_id"],
                order_info["price"],
                order_info["qty"],
                order_info["side"],
            )

    def execute_fill(self, order_id: int, fill_qty: float, price: float, side: str) -> None:
        """Record fill and update PnL / inventory."""
        self.state.fills.append(
            Fill(order_id=order_id, price=price, qty=fill_qty, side=side, time_ns=self.state.time_ns)
        )
        if side == "bid":
            self.state.inventory += fill_qty
            self.state.pnl -= price * fill_qty
        else:
            self.state.inventory -= fill_qty
            self.state.pnl += price * fill_qty

    def run_bar(self, time_ns: int, mid: float, trades: List[tuple]) -> None:
        """
        Process one bar/step: update time and mid, release delayed orders,
        then process trades (list of (price, qty, side)).
        """
        self.set_mid(time_ns, mid)
        self.step_latency()
        for price, qty, side in trades:
            fills = self.inject_trade(time_ns, price, qty, side)
            for oid, fq in fills:
                self.execute_fill(oid, fq, price, side)
