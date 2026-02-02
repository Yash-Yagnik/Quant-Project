#pragma once

#include "lumina/types.hpp"

namespace lumina {

/// Order Book Imbalance (OBI): (bid_vol - ask_vol) / (bid_vol + ask_vol).
/// Range [-1, 1]. Positive => more bid volume => short-term upward pressure.
inline double order_book_imbalance(Qty bid_volume, Qty ask_volume) {
  Qty total = bid_volume + ask_volume;
  if (total == 0) return 0.0;
  return static_cast<double>(bid_volume - ask_volume) / static_cast<double>(total);
}

/// Smoothed OBI (EMA) for stability.
class OBISignal {
public:
  explicit OBISignal(double alpha = 0.1) : alpha_(alpha), ema_(0.0) {}

  double update(Qty bid_volume, Qty ask_volume) {
    double raw = order_book_imbalance(bid_volume, ask_volume);
    ema_ = alpha_ * raw + (1.0 - alpha_) * ema_;
    return ema_;
  }

  double value() const { return ema_; }
  void reset() { ema_ = 0.0; }

private:
  double alpha_;
  double ema_;
};

} // namespace lumina
