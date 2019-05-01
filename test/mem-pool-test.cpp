#include <gtest/gtest.h>
#include "mem-pool.h"

namespace topN_freq {

TEST(MemTrackerTest, Basic) {
  MemTracker mem_tracker(100);
  EXPECT_FALSE(mem_tracker.TryConsume(101));
  EXPECT_TRUE(mem_tracker.TryConsume(100));

  mem_tracker.Release(100);
  mem_tracker.Consume(101);
  EXPECT_FALSE(mem_tracker.TryConsume(1));
  mem_tracker.Release(2);
  EXPECT_TRUE(mem_tracker.TryConsume(1));
}

TEST(MemPoolTest, AllocAndFree) {
  MemTracker mem_tracker(100);
  MemPool pool(&mem_tracker);

  EXPECT_TRUE(pool.TryAllocate(101) == nullptr);
  char* addr = pool.TryAllocate(100);
  EXPECT_TRUE(addr != nullptr);
  EXPECT_TRUE(pool.TryAllocate(1) == nullptr);

  pool.Free(addr);
  addr = pool.TryAllocate(1);
  EXPECT_TRUE(addr != nullptr);
}

TEST(MemPoolTest, BasicAllocation) {
  MemTracker mem_tracker(100);
  MemPool pool(&mem_tracker);
  for (int i = 0; i < 10; ++i) {
    char* ptr = pool.TryAllocate(10);
    EXPECT_TRUE(ptr != nullptr);
  }
  EXPECT_TRUE(pool.TryAllocate(1) == nullptr);
  EXPECT_TRUE(pool.Allocate(1) != nullptr);
}
} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}