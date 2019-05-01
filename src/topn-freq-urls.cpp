#include <vector>
#include <queue>
#include <stack>
#include <iostream>

#include "topn-freq-urls.h"

using namespace std;

namespace topN_freq {

TopNFreqUrls::~TopNFreqUrls() {
  file_mgr_.Close();
}

void TopNFreqUrls::AggregateAndSpillToDisk(std::vector<StringVal>& urls) {
  file_mgr_.NewOutputFile();
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
      file_mgr_.WriteResultTuple(curr_tuple);
      curr_tuple.Init(url);
    }
  }
  if (!curr_tuple.IsEmpty()) file_mgr_.WriteResultTuple(curr_tuple);

  // Clear the memory pool after spill
  for (StringVal& url : urls) mem_pool_.Free(url.ptr);
  urls.clear();
}

void TopNFreqUrls::Run() {
  file_mgr_.Open();
  StringVal str;
  std::vector<StringVal> urls;
  while (file_mgr_.ReadLine(MAX_URL_LEN, url_buf_, &str.len)) {
    str.ptr = mem_pool_.TryAllocate(str.len);
    // Spill to disk if  MemPool is full
    if (str.ptr == nullptr) AggregateAndSpillToDisk(urls);
    str.ptr = mem_pool_.TryAllocate(str.len);
    // DCHECK(str.ptr != nullptr);
    memcpy(str.ptr, url_buf_, str.len);
    urls.push_back(str);
  }
  if (!urls.empty()) AggregateAndSpillToDisk(urls);
  file_mgr_.Close();

  // From now on we don't have memory pressure, so we don't need to track the memory
  // anymore.
  std::priority_queue<ResultTuple> topN_tuples;
  file_merger_.Open(file_mgr_.TotalOutputFiles());
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
      topN_tuples.push(curr_tuple);
      while (topN_tuples.size() > num_results_) {
        mem_pool_.Free(topN_tuples.top().str.ptr);
        topN_tuples.pop();
      }
      curr_tuple = next_tuple;
    }
  }
  if (!curr_tuple.IsEmpty()) {
    topN_tuples.push(curr_tuple);
    while (topN_tuples.size() > num_results_) {
      mem_pool_.Free(topN_tuples.top().str.ptr);
      topN_tuples.pop();
    }
  }

  std::stack<ResultTuple> results;
  while (!topN_tuples.empty()) {
    results.push(topN_tuples.top());
    topN_tuples.pop();
  }
  while (!results.empty()) {
    ResultTuple tuple = results.top();
    cout << tuple.toString() << endl;
    results.pop();
  }
}
}