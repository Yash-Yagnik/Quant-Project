#include <gtest/gtest.h>
#include <cstring>
#include "lumina/fix_engine.hpp"

using namespace lumina;

TEST(FixEngine, ParseNewOrderSingle) {
  FixEngine fix;
  OrderId received_id = 0;
  Price received_px = 0;
  Qty received_qty = 0;
  Side received_side = Side::Buy;
  fix.set_order_callback([&](OrderId id, Price px, Qty qty, Side s) {
    received_id = id;
    received_px = px;
    received_qty = qty;
    received_side = s;
  });
  const char* msg = "35=D|11=12345|55=AAPL|54=1|40=2|44=15000|38=100|59=1|";
  ASSERT_TRUE(fix.parse(msg, std::strlen(msg)));
  EXPECT_EQ(received_id, 12345u);
  EXPECT_EQ(received_px, 15000);
  EXPECT_EQ(received_qty, 100);
  EXPECT_EQ(received_side, Side::Buy);
}

TEST(FixEngine, BuildNewOrderSingle) {
  FixEngine fix;
  std::string msg = fix.build_new_order_single(1, "AAPL", Side::Sell, 50, 20000);
  EXPECT_TRUE(msg.find("35=D") != std::string::npos);
  EXPECT_TRUE(msg.find("11=1") != std::string::npos);
  EXPECT_TRUE(msg.find("44=20000") != std::string::npos);
}
