#include <gtest/gtest.h>
#include "lumina/order_book.hpp"

using namespace lumina;

TEST(OrderBook, AddCancel) {
  OrderBook book(1024);
  ASSERT_TRUE(book.add_order(1, 100, 10, Side::Buy));
  ASSERT_TRUE(book.add_order(2, 101, 20, Side::Buy));
  ASSERT_TRUE(book.add_order(3, 99, 5, Side::Sell));
  ASSERT_EQ(book.best_bid(), 101);
  ASSERT_EQ(book.best_ask(), 99);
  book.cancel_order(2);
  ASSERT_EQ(book.best_bid(), 100);
}

TEST(OrderBook, MidAndVolume) {
  OrderBook book(1024);
  book.add_order(1, 10000, 100, Side::Buy);
  book.add_order(2, 10010, 100, Side::Sell);
  ASSERT_EQ(book.mid_price(), 10005);
  Qty bv = 0, av = 0;
  book.get_bid_ask_volumes(bv, av);
  ASSERT_EQ(bv, 100);
  ASSERT_EQ(av, 100);
}
