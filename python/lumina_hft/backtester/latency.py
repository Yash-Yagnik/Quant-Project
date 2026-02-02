"""
Tick-to-trade latency simulation.

Penalizes strategy by adding X microseconds of delay between signal and execution.
In backtest: order is not sent until (signal_time + latency_us).
"""

from dataclasses import dataclass
from typing import Optional
import numpy as np


@dataclass
class LatencyModel:
    """
    Latency in microseconds. Can be fixed or random (e.g. log-normal).
    """

    mean_us: float
    std_us: float = 0.0
    rng: Optional[np.random.Generator] = None

    def __post_init__(self):
        if self.rng is None:
            self.rng = np.random.default_rng()

    def sample_us(self) -> float:
        """Sample latency in microseconds."""
        if self.std_us <= 0:
            return max(0.0, self.mean_us)
        # Log-normal style: ensure positive
        x = self.rng.normal(self.mean_us, self.std_us)
        return max(0.0, x)

    def delay_seconds(self) -> float:
        """Convert sampled latency to seconds."""
        return self.sample_us() * 1e-6


class TickToTrade:
    """
    Tracks pending orders that are delayed by latency.
    When backtest time reaches (signal_time + latency), order is released.
    """

    def __init__(self, latency_model: LatencyModel):
        self.latency = latency_model
        self.pending: list = []  # (release_time, order_info)

    def submit(self, current_time_ns: int, order_info: dict) -> None:
        """Submit order with latency; it will be released at release_time."""
        latency_sec = self.latency.delay_seconds()
        release_ns = current_time_ns + int(latency_sec * 1e9)
        self.pending.append((release_ns, order_info))

    def release_ready(self, current_time_ns: int) -> list:
        """Return list of orders that are ready to execute (release_time <= current_time)."""
        ready = [o for (t, o) in self.pending if t <= current_time_ns]
        self.pending = [(t, o) for (t, o) in self.pending if t > current_time_ns]
        return ready
