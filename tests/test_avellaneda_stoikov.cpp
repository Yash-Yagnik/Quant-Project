#include <gtest/gtest.h>
#include "lumina/avellaneda_stoikov.hpp"
#include <cmath>

using namespace lumina;

TEST(AvellanedaStoikov, ReservationPrice) {
  AvellanedaStoikov as(0.1, 0.02, 3600.0);
  double s = 100.0, t = 0.0, q = 10.0;
  double r = as.reservation_price(s, t, q);
  double expected = s - q * 0.1 * 0.02 * 0.02 * 3600.0;
  EXPECT_NEAR(r, expected, 1e-9);
}

TEST(AvellanedaStoikov, QuotesWithOBI) {
  AvellanedaStoikov as(0.1, 0.02, 3600.0);
  double bid_off, ask_off;
  as.get_quotes(100.0, 0.0, 0.0, 1.5, 0.0, bid_off, ask_off);
  EXPECT_LT(bid_off, 100.0);
  EXPECT_GT(ask_off, 100.0);
  EXPECT_LT(bid_off, ask_off);
}
