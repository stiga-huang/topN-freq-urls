#include <gtest/gtest.h>
#include "mem-pool.h"
#include "file-manager.h"

using namespace std;

namespace topN_freq {

TEST(FileManagerTest, SpillFileName) {
  EXPECT_EQ(FileManager::OutputFileName("base", 0), "base.out.0");
  EXPECT_EQ(FileManager::OutputFileName("base", 1), "base.out.1");
  EXPECT_EQ(FileManager::OutputFileName("base", 2), "base.out.2");

  EXPECT_EQ(FileManager::OutputFileName("/tmp/input", 2), "/tmp/input.out.2");
}

TEST(FileManagerTest, ReadLine) {
  string urls[] = {"http://www.google.com", "http://www.facebook.com", "abc"};
  istringstream input(urls[0] + '\n' + urls[1] + '\n' + urls[2] + "\n\n\n");
  char buffer[MAX_URL_LEN];
  size_t url_len;

  for (int i = 0; i < 3; ++i) {
    EXPECT_TRUE(FileManager::ReadLine(input, MAX_URL_LEN, buffer, &url_len));
    EXPECT_EQ(url_len, urls[i].length());
    EXPECT_EQ(string(buffer, url_len), urls[i]);
  }
  EXPECT_FALSE(FileManager::ReadLine(input, MAX_URL_LEN, buffer, &url_len));
}

TEST(FileMergerTest, ReadTuple) {
  MemTracker mem_tracker(MAX_URL_LEN * 100);
  MemPool pool(&mem_tracker);
  istringstream file("http://abc\t1\nhttp://bbbb\t100\n");
  ResultTuple res;

  EXPECT_TRUE(FileMerger::NextResultTupleInFile(&file, &pool, &res));
  EXPECT_EQ(res.str.toString(), "http://abc");
  EXPECT_EQ(res.cnt, 1);

  EXPECT_TRUE(FileMerger::NextResultTupleInFile(&file, &pool, &res));
  EXPECT_EQ(res.str.toString(), "http://bbbb");
  EXPECT_EQ(res.cnt, 100);
}

TEST(FileMergerTest, SortTuple) {
  MemTracker mem_tracker(MAX_URL_LEN * 100);
  MemPool pool(&mem_tracker);
  char files[][100] = {
      "abc\t1\nbbb\t2\ndddd\t100\n",
      "bbb\t10\ndddd\t2",
      "abc\t4\ndddd\t10\n"
  };
  vector<istream*> inputs;
  for (int i = 0; i < 3; ++i) {
    inputs.push_back(new istringstream(files[i]));
  }
  FileMerger merger(inputs, &pool);
  merger.Open(3);

  char expected_strs[][10] = {
      "abc", "abc", "bbb", "bbb", "dddd", "dddd", "dddd"
  };
  int expected_cnts[] = {
      1, 4, 2, 10, 2, 10, 100
  };
  ResultTuple res;
  for (int i = 0; i < sizeof(expected_cnts) / sizeof(int); ++i) {
    merger.NextResultTuple(&res);
    EXPECT_EQ(res.str.toString(), expected_strs[i]);
    EXPECT_EQ(res.cnt, expected_cnts[i]);
  }
}
}