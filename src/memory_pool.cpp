#include "lumina/memory_pool.hpp"
#include <stdexcept>

namespace lumina {

OrderNodePool::OrderNodePool(size_t max_orders) : capacity_(max_orders) {
  block_.resize(max_orders);
  free_list_.resize(max_orders);
  for (size_t i = 0; i < max_orders; ++i)
    free_list_[i] = &block_[i];
  free_idx_ = max_orders;
}

OrderNodePool::~OrderNodePool() = default;

OrderNodePool::Node* OrderNodePool::allocate() {
  if (free_idx_ == 0) return nullptr;
  Node* n = free_list_[--free_idx_];
  n->next = n->prev = nullptr;
  ++used_;
  return n;
}

void OrderNodePool::deallocate(Node* n) {
  if (!n) return;
  if (free_idx_ < capacity_)
    free_list_[free_idx_++] = n;
  --used_;
}

} // namespace lumina
