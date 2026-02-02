#include <gtest/gtest.h>
#include "lumina/ring_buffer.hpp"
#include "lumina/types.hpp"

using namespace lumina;

TEST(RingBuffer, SPSCPushPop) {
  SPSCRingBuffer<int, 16> rb;
  ASSERT_TRUE(rb.empty());
  for (int i = 0; i < 16; ++i)
    ASSERT_TRUE(rb.try_push(i));
  ASSERT_FALSE(rb.try_push(99));
  int x;
  for (int i = 0; i < 16; ++i) {
    ASSERT_TRUE(rb.try_pop(x));
    ASSERT_EQ(x, i);
  }
  ASSERT_FALSE(rb.try_pop(x));
}

TEST(RingBuffer, MPMCPushPop) {
  MPMCRingBuffer<MarketDataEvent, 16> rb;
  MarketDataEvent e{};
  e.mid = 100;
  ASSERT_TRUE(rb.try_push(e));
  MarketDataEvent out;
  ASSERT_TRUE(rb.try_pop(out));
  ASSERT_EQ(out.mid, 100);
}
