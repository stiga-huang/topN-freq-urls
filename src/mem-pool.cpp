#include "mem-pool.h"

namespace topN_freq {

MemPool::~MemPool() {
  for (auto it = chunk_sizes_.begin(); it != chunk_sizes_.end(); ++it) {
    std::free(it->first);
    mem_tracker_->Release(it->second);
  }
  chunk_sizes_.clear();
}

char* MemPool::TryAllocate(uint64_t size) {
  if (!mem_tracker_->TryConsume(size)) return nullptr;
  char* addr = reinterpret_cast<char*>(std::malloc(size));
  chunk_sizes_[addr] = size;
  return addr;
}

char* MemPool::Allocate(uint64_t size) {
  mem_tracker_->Consume(size);
  char* addr = reinterpret_cast<char*>(std::malloc(size));
  chunk_sizes_[addr] = size;
  return addr;
}

void MemPool::Free(char* ptr) {
  if (chunk_sizes_.find(ptr) == chunk_sizes_.end()) {
    // TODO: raise Exception?
    return;
  }
  mem_tracker_->Release(chunk_sizes_[ptr]);
  chunk_sizes_.erase(ptr);
  std::free(ptr);
}
}
