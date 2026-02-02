#include <gtest/gtest.h>
#include "lumina/memory_pool.hpp"

using namespace lumina;

TEST(MemoryPool, TemplatePoolAllocFree) {
  MemoryPool<int, 64> pool;
  std::vector<int*> ptrs;
  for (size_t i = 0; i < 64; ++i) {
    int* p = pool.allocate();
    ASSERT_NE(p, nullptr);
    *p = static_cast<int>(i);
    ptrs.push_back(p);
  }
  ASSERT_EQ(pool.allocate(), nullptr);
  for (int* p : ptrs)
    pool.deallocate(p);
  for (size_t i = 0; i < 64; ++i)
    ASSERT_NE(pool.allocate(), nullptr);
}

TEST(MemoryPool, OrderNodePool) {
  OrderNodePool pool(100);
  std::vector<OrderNodePool::Node*> nodes;
  for (size_t i = 0; i < 100; ++i) {
    auto* n = pool.allocate();
    ASSERT_NE(n, nullptr);
    nodes.push_back(n);
  }
  ASSERT_EQ(pool.allocate(), nullptr);
  for (auto* n : nodes)
    pool.deallocate(n);
  ASSERT_EQ(pool.size_used(), 0u);
}
