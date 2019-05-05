#include <gtest/gtest.h>

#include "data.h"
#include "mem-pool.h"

namespace topN_freq {

void InitStringVal(const char* str, StringVal* res) {
  res->ptr = const_cast<char*>(str);
  res->len = strlen(str);
}

void InitResultTuple(const char* str, int cnt, ResultTuple* res) {
  InitStringVal(str, &res->str);
  res->cnt = cnt;
}

TEST(StringValTest, Order) {
  StringVal a, b;
  InitStringVal("aaa", &a);
  InitStringVal("bbb", &b);
  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);

  InitStringVal("aaa", &a);
  InitStringVal("aaab", &b);
  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);
}

TEST(StringValTest, Equal) {
  StringVal a, b;
  InitStringVal("aaa", &a);
  InitStringVal("aaa", &b);
  EXPECT_TRUE(a.StrEquals(b));
}

TEST(StringValTest, Compare) {
  StringVal a, b;
  InitStringVal("abc", &a);
  InitStringVal("bbb", &b);
  EXPECT_TRUE(a.StrCompare(b) < 0);
  EXPECT_TRUE(b.StrCompare(a) > 0);

  InitStringVal("abc", &a);
  InitStringVal("abcd", &b);
  EXPECT_TRUE(a.StrCompare(b) < 0);
  EXPECT_TRUE(b.StrCompare(a) > 0);

  EXPECT_TRUE(a.StrCompare(a) == 0);
}

TEST(ResultTupleTest, Order) {
  ResultTuple t1, t2;
  InitResultTuple("aaa", 1, &t1);
  InitResultTuple("bbb", 2, &t2);
  // The worst the larger. So will be on the top of the priority queue.
  EXPECT_FALSE(t1 < t2);
  EXPECT_TRUE(t2 < t1);

  InitResultTuple("aaa", 1, &t1);
  InitResultTuple("bbb", 1, &t2);
  EXPECT_TRUE(t1 < t2);
  EXPECT_FALSE(t2 < t1);
}

TEST(ResultTupleTest, StrEqual) {
  ResultTuple t1, t2;
  InitResultTuple("aaa", 1, &t1);
  InitResultTuple("aaa", 2, &t2);
  EXPECT_TRUE(t1.StrEquals(t2));
}

TEST(StringLessTest, Basic) {
  StringLess comparator;
  ResultTuple t1, t2;
  InitResultTuple("aaa", 1, &t1);
  InitResultTuple("bbb", 2, &t2);
  EXPECT_FALSE(comparator(t1, t2));
  EXPECT_TRUE(comparator(t2, t1));
}
}