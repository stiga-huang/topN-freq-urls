#include <algorithm>
#include <vector>
#include <cstring>
#include <ctime>
#include <queue>
#include <stack>
#include <iostream>

#include "topn-freq-urls.h"

using namespace std;

#define RETURN_ON_FALSE(stmt)                          \
  do {                                                 \
    if (!stmt) return false;                           \
  } while (false)

namespace topN_freq {

TopNFreqUrls::~TopNFreqUrls() {
  file_mgr_.Close();
}

bool TopNFreqUrls::AggregateAndSpillToDisk(std::vector<StringVal>& urls) {
  RETURN_ON_FALSE(file_mgr_.NewOutputFile());
  sort(urls.begin(), urls.end());

  ResultTuple curr_tuple;
  curr_tuple.SetEmpty();
  for (StringVal& url : urls) {
    if (curr_tuple.IsEmpty()) {
      curr_tuple.Init(url);
      continue;
    }
    if (curr_tuple.str.StrEquals(url)) {
      curr_tuple.cnt++;
    } else {
      RETURN_ON_FALSE(file_mgr_.WriteResultTuple(curr_tuple));
      curr_tuple.Init(url);
    }
  }
  if (!curr_tuple.IsEmpty()) {
    RETURN_ON_FALSE(file_mgr_.WriteResultTuple(curr_tuple));
  }

  // Clear the memory pool after spill
  for (StringVal& url : urls) mem_pool_.Free(url.ptr);
  urls.clear();
  return true;
}

bool TopNFreqUrls::PartialSort() {
  RETURN_ON_FALSE(file_mgr_.Open());
  StringVal str;
  std::vector<StringVal> urls;
  while (file_mgr_.ReadLine(MAX_URL_LEN, url_buf_, &str.len)) {
    str.ptr = mem_pool_.TryAllocate(str.len);
    // Spill to disk if MemPool is full
    if (str.ptr == nullptr) {
      RETURN_ON_FALSE(AggregateAndSpillToDisk(urls));
      str.ptr = mem_pool_.TryAllocate(str.len);
    }
    // DCHECK(str.ptr != nullptr);
    memcpy(str.ptr, url_buf_, str.len);
    urls.push_back(str);
  }
  if (!urls.empty()) {
    RETURN_ON_FALSE(AggregateAndSpillToDisk(urls));
  }
  return file_mgr_.Close();
}

inline void TopNFreqUrls::UpdateHeap(ResultTuple tuple,
    std::priority_queue<ResultTuple>* heap) {
  heap->push(tuple);
  while (heap->size() > num_results_) {
    mem_pool_.Free(heap->top().str.ptr);
    heap->pop();
  }
}

bool TopNFreqUrls::MergeSort(std::vector<ResultTuple>* results) {
  // From now on we don't have memory pressure, so we don't need to track the memory
  // anymore. We maintain a min heap containing the topN results. The url with less
  // occurrence will be on the heap top.
  std::priority_queue<ResultTuple> topN_tuples;
  RETURN_ON_FALSE(file_merger_.Open(file_mgr_.TotalOutputFiles()));
  ResultTuple curr_tuple;
  ResultTuple next_tuple;
  curr_tuple.SetEmpty();
  while (file_merger_.NextResultTuple(&next_tuple)) {
    if (curr_tuple.IsEmpty()) {
      curr_tuple = next_tuple;
      continue;
    }
    if (curr_tuple.StrEquals(next_tuple)) {
      curr_tuple.cnt += next_tuple.cnt;
      mem_pool_.Free(next_tuple.str.ptr);
    } else {
      UpdateHeap(curr_tuple, &topN_tuples);
      curr_tuple = next_tuple;
    }
  }
  if (!curr_tuple.IsEmpty()) {
    UpdateHeap(curr_tuple, &topN_tuples);
  }
  file_merger_.Close();
  if (topN_tuples.empty()) {
    cout << "No urls scanned!" << endl;
    return true;
  }
  results->resize(topN_tuples.size());
  for (int i = topN_tuples.size() - 1; i >= 0; --i) {
    (*results)[i] = topN_tuples.top();
    topN_tuples.pop();
  }
  return true;
}

void TopNFreqUrls::Run() {
  clock_t spill_begin = clock();
  if (!PartialSort()) return; // exit on errors
  clock_t spill_end = clock();
  vector<ResultTuple> results;
  MergeSort(&results);
  clock_t merge_end = clock();

  for (ResultTuple tuple : results) {
    cout << tuple.toString() << endl;
  }
  cout << "Spill stage: " << double(spill_end - spill_begin) / CLOCKS_PER_SEC
       << "s" << endl
       << "Merge stage: " << double(merge_end - spill_end) / CLOCKS_PER_SEC
       << "s" << endl;
}
}
