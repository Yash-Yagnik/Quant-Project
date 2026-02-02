#!/usr/bin/env python3
"""Run HFT backtester with queue sim, latency, slippage."""

import sys
import yaml
from pathlib import Path

# Add python package to path
sys.path.insert(0, str(Path(__file__).resolve().parent))

from lumina_hft.backtester import BacktestEngine, BacktestConfig, QueueSimLOB
from lumina_hft.avellaneda_stoikov import reservation_price, get_quotes, order_book_imbalance


def load_config(path: str) -> dict:
    with open(path) as f:
        return yaml.safe_load(f)


def main():
    repo_root = Path(__file__).resolve().parent.parent
    config_path = repo_root / "config" / "backtest.yaml"
    if not config_path.exists():
        cfg = {
            "tick_size": 0.01,
            "latency_mean_us": 50,
            "latency_std_us": 10,
            "impact_eta": 0.1,
            "impact_V": 1e6,
            "spread_bps": 5,
            "seed": 42,
        }
    else:
        cfg = load_config(str(config_path))

    bt_cfg = BacktestConfig(
        tick_size=cfg.get("tick_size", 0.01),
        latency_mean_us=cfg.get("latency_mean_us", 50),
        latency_std_us=cfg.get("latency_std_us", 10),
        impact_eta=cfg.get("impact_eta", 0.1),
        impact_V=cfg.get("impact_V", 1e6),
        spread_bps=cfg.get("spread_bps", 5),
        seed=cfg.get("seed"),
    )
    engine = BacktestEngine(bt_cfg)

    # Simple scenario: mid = 100, inject a few trades
    engine.set_mid(0, 100.0)
    engine.submit_order(1, 99.5, 10, "bid")
    engine.set_mid(1_000_000, 100.0)
    engine.step_latency()
    # Trade through bid side (someone sells) -> our bid could get hit
    fills = engine.inject_trade(2_000_000, 99.5, 15, "ask")
    for oid, fq in fills:
        engine.execute_fill(oid, fq, 99.5, "bid")

    print("Fills:", len(engine.state.fills))
    for f in engine.state.fills:
        print(f"  {f.order_id} @ {f.price} x {f.qty} {f.side}")
    print("Inventory:", engine.state.inventory)
    print("PnL:", engine.state.pnl)
    print("Backtest OK.")


if __name__ == "__main__":
    main()
