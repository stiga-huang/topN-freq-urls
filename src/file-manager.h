#ifndef TOPN_FREQ_URLS_FILEMANAGER_H
#define TOPN_FREQ_URLS_FILEMANAGER_H

#include <fstream>
#include <queue>
#include <vector>

#include "mem-pool.h"
#include "data.h"

namespace topN_freq {

const unsigned int MAX_URL_LEN = 1024;

/// Manager for the input file and spill files.
class FileManager {
 public:
  FileManager(const char* file_name): file_name_(file_name) {}
  ~FileManager();

  bool Open();
  bool Close();
  bool NewOutputFile();
  bool ReadLine(unsigned int max_len, char* buffer, size_t* str_len);
  void WriteResultTuple(ResultTuple& res);
  int TotalOutputFiles() { return output_files_; }

  /// Read a URL (separated by '\n' by defaults) into the 'buffer' and return the
  /// URL length in 'str_len'. Return false if no more data to read.
  static bool ReadLine(std::istream& in, unsigned int max_len, char* buffer,
      size_t* str_len, char delimiter = '\n');

  /// Get the spill file name. The have the suffix as '.out.0', '.out.1', etc.
  static std::string OutputFileName(const char* base_file_name, int file_index);
 private:
  const char* file_name_;
  std::ifstream in_;
  std::ofstream out_;
  int output_files_ = 0;
};

/// Merger for the spill files.
class FileMerger {
 public:
  FileMerger(const char* base_file_name, MemPool* mem_pool)
      : base_file_name_(base_file_name), mem_pool_(mem_pool) {}

  /// Constructor used in test
  FileMerger(std::vector<std::istream*>& inputs, MemPool* mem_pool)
      : mem_pool_(mem_pool) {
    sorted_files_.assign(inputs.begin(), inputs.end());
    test_mode = true;
  }

  bool Open(int num_files);
  bool Close();
  bool NextResultTuple(ResultTuple* res);
  /// Read a ResultTuple from input stream 'in' into 'res'.
  static bool NextResultTupleInFile(std::istream* in, MemPool* mem_pool, ResultTuple* res);
 private:
  bool OpenInternal(int num_files);

  const char* base_file_name_;
  MemPool* mem_pool_;
  std::vector<std::istream*> sorted_files_;
  /// Drained labels for each file
  std::vector<bool> drained_;
  /// A min heap based on the order of the strings (urls). This is the order inside the
  /// sorted file.
  std::priority_queue<ResultTuple, std::vector<ResultTuple>, StringLess> res_tuples_heap_;
  /// In test mode, the 'sorted_files' are provided by strings using the second constructor.
  /// We don't need to call 'OpenInternal' to open files.
  bool test_mode = false;
};
}
#endif //TOPN_FREQ_URLS_FILEMANAGER_H
