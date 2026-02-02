# Lumina-HFT: Multi-Threaded Liquidity Provision Framework

A full-stack HFT system combining a high-performance C++20/23 execution core with a Python-based research and backtesting environment.

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     Python Research Layer                        │
│  Avellaneda-Stoikov Analysis │ Backtester │ KDB+ / Mock          │
└─────────────────────────────┬───────────────────────────────────┘
                              │ pybind11
┌─────────────────────────────▼───────────────────────────────────┐
│                     C++ Low-Latency Core                         │
│  Strategy Engine ←→ Ring Buffer ←→ Market Data Handler           │
│  Order Book (Lock-Free) │ Memory Pool │ Thread Pinning           │
└─────────────────────────────┬───────────────────────────────────┘
                              │ FIX / TCP
┌─────────────────────────────▼───────────────────────────────────┐
│                     Exchange / Simulator                         │
└─────────────────────────────────────────────────────────────────┘
```

## Features

- **Lock-Free Order Book**: O(1) price-level lookups, doubly-linked lists for time priority
- **Custom Memory Pool**: Zero malloc/new on the hot path
- **Thread Pinning & CPU Isolation**: `pthread_setaffinity_np`, `isolcpus`-aware
- **Disruptor-Style Ring Buffer**: SPSC/MPMC for market data → strategy
- **Avellaneda-Stoikov Market Making**: Reservation price + Order Book Imbalance (OBI)
- **HFT Backtester**: Queue position, tick-to-trade latency, square-root impact
- **SIMD (AVX-512)**: Fast indicator calculations
- **Pre-Trade Risk**: Notional and fat-finger limits
- **FIX Engine**: Basic FIX protocol for order entry
- **KDB+/q Mock**: Time-series tick storage

## Build (CMake)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
ctest
```

For CPU isolation (recommended for lowest latency), add to kernel cmdline: `isolcpus=1,2,3` and pin threads via `lumina::pin_thread_to_core(core_id)`.

## Python

```bash
pip install -r requirements.txt
# Run backtester
python -m lumina_hft.backtester --config config/backtest.yaml
```

## Docker

```bash
docker build -t lumina-hft .
docker run --rm lumina-hft ./tests/run_tests
```

## License

MIT
