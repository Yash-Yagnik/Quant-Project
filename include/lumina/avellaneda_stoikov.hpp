#pragma once

#include "lumina/types.hpp"
#include <cmath>

namespace lumina {

/// Avellaneda-Stoikov market making: reservation price and spread.
/// r(s,t,q) = s - q * gamma * sigma^2 * (T - t)
/// Optimal spread depends on gamma and sigma.
class AvellanedaStoikov {
public:
  AvellanedaStoikov(double gamma, double sigma, double T_seconds)
    : gamma_(gamma), sigma_(sigma), T_(T_seconds) {}

  /// Reservation price: mid adjusted for inventory risk.
  /// s = mid, q = inventory (positive = long), t = current time in seconds.
  double reservation_price(double s, double t, double q) const {
    double tau = T_ - t;
    if (tau <= 0) return s;
    return s - q * gamma_ * sigma_ * sigma_ * tau;
  }

  /// Optimal half-spread (symmetric around reservation price).
  double optimal_half_spread(double k) const {
    return (1.0 / k) * std::log(1.0 + gamma_ / k);
  }

  /// Bid/ask around reservation price with optional OBI skew.
  void get_quotes(double s, double t, double q, double k,
                   double obi_skew,  // -1 to +1: negative = more bids, positive = more asks
                   double& bid_offset, double& ask_offset) const {
    double r = reservation_price(s, t, q);
    double half = optimal_half_spread(k);
    // OBI: if bid side has more volume, expect upward move -> skew quotes up
    double skew = obi_skew * 0.5 * half;  // configurable weight
    bid_offset = r - half - skew;
    ask_offset = r + half + skew;
  }

  void set_sigma(double sigma) { sigma_ = sigma; }
  void set_gamma(double gamma) { gamma_ = gamma; }
  void set_T(double T_seconds) { T_ = T_seconds; }

private:
  double gamma_;
  double sigma_;
  double T_;
};

} // namespace lumina
