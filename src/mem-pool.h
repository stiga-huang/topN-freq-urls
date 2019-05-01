#ifndef TOPN_FREQ_URLS_MEM_POOL_H
#define TOPN_FREQ_URLS_MEM_POOL_H

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace topN_freq {

class MemTracker {
 public:
  MemTracker(uint64_t max_size) : max_size_(max_size) {}

  /// Try to consume 'size' space. Return false if we'll excceed the mem limit.
  bool TryConsume(uint64_t size) {
    if (current_size_ + size > max_size_) return false;
    current_size_ += size;
    return true;
  }

  /// Consume 'size' space anyway (ignoring the mem limit).
  void Consume(uint64_t size) {
    current_size_ += size;
  }

  void Release(uint64_t size) {
    current_size_ -= size;
  }

  uint64_t GetCurrentSize() { return current_size_; }
 private:
  uint64_t max_size_;
  uint64_t current_size_ = 0;
};

class MemPool {
 public:
  MemPool(MemTracker* mem_tracker) : mem_tracker_(mem_tracker) {}
  ~MemPool();

  /// Try to allocate spaces. Return nullptr if the pool is full.
  char* TryAllocate(uint64_t size);

  /// Allocate spaces ignoring the remaining space inside the pool.
  char* Allocate(uint64_t size);

  void Free(char* ptr);

 private:
  MemTracker* mem_tracker_;
  std::unordered_map<char*, uint64_t> chunk_sizes_;
};

template<class T>
class Allocator : public std::allocator<T>{
 public:
  Allocator(MemTracker* mem_tracker): std::allocator<T>(), mem_tracker_(mem_tracker) {}
  virtual ~Allocator() {}

  T* allocate(size_t n, const void *hint=0) {
    mem_tracker_->Consume(n);
    return std::allocator<T>::allocate(n, hint);
  }

  void deallocate(T* p, size_t n) {
    mem_tracker_->Release(n);
    std::allocator<T>::deallocate(p, n);
  }
 private:
  MemTracker* mem_tracker_;
};
}
#endif //TOPN_FREQ_URLS_MEM_POOL_H
