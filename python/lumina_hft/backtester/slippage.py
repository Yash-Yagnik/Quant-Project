"""
Market impact and slippage: square-root law.

Impact = eta * sqrt(Q / V) where Q = order size, V = typical volume.
Used to move mid when we execute a trade in the backtester.
"""

import numpy as np
from typing import Tuple


def square_root_impact(
    qty: float,
    side: str,
    eta: float = 0.1,
    V: float = 1e6,
) -> float:
    """
    Price impact in same units as mid (e.g. dollars).

    impact = eta * sqrt(Q / V)
    For a buy, mid moves up by impact; for a sell, mid moves down.

    Parameters
    ----------
    qty : order quantity
    side : "buy" or "sell"
    eta : impact coefficient
    V : reference daily volume (same units as qty)
    """
    if qty <= 0 or V <= 0:
        return 0.0
    impact = eta * np.sqrt(qty / V)
    return impact if side == "sell" else -impact


def apply_slippage(
    mid: float,
    qty: float,
    side: str,
    eta: float = 0.1,
    V: float = 1e6,
    spread_bps: float = 5.0,
) -> Tuple[float, float]:
    """
    Execution price = mid + spread_half + impact.
    Returns (execution_price, impact).
    """
    impact = square_root_impact(qty, side, eta, V)
    spread_half = mid * (spread_bps / 10000.0) / 2
    if side == "buy":
        exec_price = mid + spread_half + abs(impact)
    else:
        exec_price = mid - spread_half - abs(impact)
    return exec_price, impact
