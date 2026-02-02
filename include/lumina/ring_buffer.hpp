#pragma once

#include "lumina/types.hpp"
#include <atomic>
#include <cstddef>
#include <new>

namespace lumina {

/// Single-Producer Single-Consumer ring buffer (Disruptor-style).
/// Cache-line padded to avoid false sharing. No locks.
template <typename T, size_t Size>
class SPSCRingBuffer {
public:
  static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");
  static constexpr size_t capacity = Size;
  static constexpr size_t mask = Size - 1;

  bool try_push(const T& item) {
    const size_t w = write_pos_.load(std::memory_order_relaxed);
    if (w - read_pos_.load(std::memory_order_acquire) >= Size)
      return false;
    buffer_[w & mask] = item;
    write_pos_.store(w + 1, std::memory_order_release);
    return true;
  }

  bool try_pop(T& item) {
    const size_t r = read_pos_.load(std::memory_order_relaxed);
    if (r >= write_pos_.load(std::memory_order_acquire))
      return false;
    item = buffer_[r & mask];
    read_pos_.store(r + 1, std::memory_order_release);
    return true;
  }

  size_t size() const {
    return write_pos_.load(std::memory_order_acquire) -
           read_pos_.load(std::memory_order_acquire);
  }

  bool empty() const { return size() == 0; }

private:
  alignas(64) std::atomic<size_t> write_pos_{0};
  alignas(64) std::atomic<size_t> read_pos_{0};
  T buffer_[Size];
};

/// Multi-Producer Multi-Consumer variant using sequence + CAS.
/// Slots are pre-allocated; producers claim sequence, consumers claim read.
template <typename T, size_t Size>
class MPMCRingBuffer {
public:
  static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");
  static constexpr size_t capacity = Size;
  static constexpr size_t mask = Size - 1;

  MPMCRingBuffer() {
    for (size_t i = 0; i < Size; ++i)
      sequences_[i].store(i, std::memory_order_relaxed);
  }

  bool try_push(const T& item) {
    const size_t pos = write_pos_.fetch_add(1, std::memory_order_acq_rel);
    if (pos - read_pos_.load(std::memory_order_acquire) >= Size) {
      write_pos_.fetch_sub(1, std::memory_order_acq_rel);
      return false;
    }
    size_t seq;
    while ((seq = sequences_[pos & mask].load(std::memory_order_acquire)) != pos)
      ; // spin until slot is free
    buffer_[pos & mask] = item;
    sequences_[pos & mask].store(pos + 1, std::memory_order_release);
    return true;
  }

  bool try_pop(T& item) {
    size_t pos = read_pos_.load(std::memory_order_acquire);
    for (;;) {
      if (pos >= write_pos_.load(std::memory_order_acquire))
        return false;
      size_t seq = sequences_[pos & mask].load(std::memory_order_acquire);
      if (seq == pos + 1) {
        if (read_pos_.compare_exchange_weak(pos, pos + 1,
                                            std::memory_order_acq_rel,
                                            std::memory_order_acquire)) {
          item = buffer_[pos & mask];
          sequences_[pos & mask].store(pos + Size, std::memory_order_release);
          return true;
        }
      } else
        pos = read_pos_.load(std::memory_order_acquire);
    }
  }

  size_t size() const {
    return write_pos_.load(std::memory_order_acquire) -
           read_pos_.load(std::memory_order_acquire);
  }

private:
  alignas(64) std::atomic<size_t> write_pos_{0};
  alignas(64) std::atomic<size_t> read_pos_{0};
  std::atomic<size_t> sequences_[Size];
  T buffer_[Size];
};

} // namespace lumina
