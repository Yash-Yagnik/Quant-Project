"""
Avellaneda-Stoikov market making model.

Reservation price: r(s,t,q) = s - q * gamma * sigma^2 * (T - t)
Optimal spread and quote placement with optional Order Book Imbalance (OBI) skew.
"""

import numpy as np
from typing import Tuple


def reservation_price(
    s: float, t: float, q: float, gamma: float, sigma: float, T: float
) -> float:
    """
    Reservation price that skews quotes based on inventory risk.

    r(s,t,q) = s - q * gamma * sigma^2 * (T - t)

    Parameters
    ----------
    s : mid price
    t : current time in seconds
    q : inventory (positive = long)
    gamma : risk aversion
    sigma : volatility
    T : session end time in seconds
    """
    tau = T - t
    if tau <= 0:
        return s
    return s - q * gamma * sigma**2 * tau


def optimal_half_spread(gamma: float, k: float) -> float:
    """Optimal half-spread: (1/k) * ln(1 + gamma/k)."""
    return (1.0 / k) * np.log(1.0 + gamma / k)


def order_book_imbalance(bid_volume: float, ask_volume: float) -> float:
    """OBI = (bid_vol - ask_vol) / (bid_vol + ask_vol), in [-1, 1]."""
    total = bid_volume + ask_volume
    if total == 0:
        return 0.0
    return (bid_volume - ask_volume) / total


def get_quotes(
    s: float,
    t: float,
    q: float,
    gamma: float,
    sigma: float,
    T: float,
    k: float,
    obi_skew: float = 0.0,
) -> Tuple[float, float]:
    """
    Bid/ask offsets around reservation price with OBI skew.

    If OBI > 0 (more bid volume), skew quotes upward (expect price increase).
    """
    r = reservation_price(s, t, q, gamma, sigma, T)
    half = optimal_half_spread(gamma, k)
    skew = obi_skew * 0.5 * half
    bid_offset = r - half - skew
    ask_offset = r + half + skew
    return bid_offset, ask_offset
