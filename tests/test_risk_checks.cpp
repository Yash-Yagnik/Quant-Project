#include <gtest/gtest.h>
#include "lumina/risk_checks.hpp"

using namespace lumina;

TEST(PreTradeRisk, AllowWithinLimits) {
  PreTradeRisk risk(1'000'000, 10'000);
  EXPECT_TRUE(risk.check_order(100, 100, Side::Buy));
  risk.add_fill(100, 100);
  EXPECT_TRUE(risk.check_order(100, 5000, Side::Sell));
}

TEST(PreTradeRisk, RejectFatFinger) {
  PreTradeRisk risk(1'000'000, 10'000);
  EXPECT_FALSE(risk.check_order(100, 20'000, Side::Buy));
}

TEST(PreTradeRisk, KillSwitch) {
  PreTradeRisk risk(1'000'000, 10'000);
  risk.kill();
  EXPECT_FALSE(risk.check_order(100, 100, Side::Buy));
  risk.reset_kill();
  EXPECT_TRUE(risk.check_order(100, 100, Side::Buy));
}
