#pragma once

#include "lumina/types.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

namespace lumina {

/// Fixed-size block pool: no malloc/new on the hot path.
/// All blocks are pre-allocated at startup; alloc/free are lock-free push/pop.
template <typename T, size_t Capacity>
class MemoryPool {
public:
  static constexpr size_t capacity = Capacity;

  MemoryPool() {
    for (size_t i = 0; i < Capacity; ++i)
      slots_[i].next.store(&slots_[i + 1], std::memory_order_relaxed);
    slots_[Capacity - 1].next.store(nullptr, std::memory_order_relaxed);
    head_.store(&slots_[0], std::memory_order_relaxed);
  }

  T* allocate() {
    Node* n = head_.load(std::memory_order_acquire);
    while (n && !head_.compare_exchange_weak(n, n->next.load(std::memory_order_relaxed),
                                             std::memory_order_acq_rel,
                                             std::memory_order_acquire))
      ;
    if (!n) return nullptr;
    n->next.store(nullptr, std::memory_order_relaxed);
    return &n->value;
  }

  void deallocate(T* p) {
    if (!p) return;
    Node* n = reinterpret_cast<Node*>(p);
    Node* old_head = head_.load(std::memory_order_acquire);
    do {
      n->next.store(old_head, std::memory_order_relaxed);
    } while (!head_.compare_exchange_weak(old_head, n,
                                          std::memory_order_acq_rel,
                                          std::memory_order_acquire));
  }

  size_t size_used() const {
    size_t count = 0;
    Node* n = head_.load(std::memory_order_acquire);
    while (n) {
      ++count;
      n = n->next.load(std::memory_order_acquire);
    }
    return Capacity - count;
  }

private:
  union Node {
    T value;
    std::atomic<Node*> next;
    Node() : next(nullptr) {}
    ~Node() {}
  };
  alignas(64) std::array<Node, Capacity> slots_;
  alignas(64) std::atomic<Node*> head_;
};

/// Heap-backed pool for runtime-sized allocation (e.g. order book nodes).
/// Still pre-allocates a contiguous block to avoid per-node malloc.
class OrderNodePool {
public:
  explicit OrderNodePool(size_t max_orders);
  ~OrderNodePool();

  struct Node {
    Order order;
    Node* next{nullptr};
    Node* prev{nullptr};
  };

  Node* allocate();
  void deallocate(Node* n);
  size_t size_used() const { return used_; }
  size_t capacity() const { return capacity_; }

private:
  std::vector<Node> block_;
  std::vector<Node*> free_list_;
  size_t capacity_{0};
  size_t used_{0};
  size_t free_idx_{0};
};

} // namespace lumina
