#ifndef TOPN_FREQ_URLS_TOPN_FREQ_URLS_H
#define TOPN_FREQ_URLS_TOPN_FREQ_URLS_H

#include <cstdio>
#include <cstdint>

#include "file-manager.h"
#include "mem-pool.h"

namespace topN_freq {

class TopNFreqUrls {
 public:
  TopNFreqUrls(const char* input_file, int num_results, uint64_t mem_limit)
      : mem_tracker_(mem_limit),
        mem_pool_(&mem_tracker_),
        num_results_(num_results),
        file_mgr_(input_file),
        file_merger_(input_file, &mem_pool_) {}
  ~TopNFreqUrls();

  void Run();
 private:
  /// Aggregate urls to get counts. Then spill the results to disks.
  /// We'll sort the urls, then count them and spill the result in a streaming
  /// manner to save memory space. The spilled file is still sorted by the url.
  /// Each time we create a new spilled file.
  void AggregateAndSpillToDisk(std::vector<StringVal>& urls);

  int num_results_;
  char url_buf_[MAX_URL_LEN + 1];
  MemTracker mem_tracker_;
  MemPool mem_pool_;
  FileManager file_mgr_;
  FileMerger file_merger_;
};
}

#endif //TOPN_FREQ_URLS_TOPN_FREQ_URLS_H
