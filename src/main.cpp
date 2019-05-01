#include <iostream>
#include "mem-pool.h"
#include "topn-freq-urls.h"

using namespace std;
using namespace topN_freq;

int main(int argc, char **argv) {
  if (argc < 3) {
    cerr << "Too few arguments!" << endl;
    cerr << "Args: file_path num_results mem_limit_in_gb" << endl;
    return 1;
  }
  const char* file_path = argv[1];
  int num_results = atoi(argv[2]);
  uint64_t mem_limit = atoi(argv[3]) * 1024L * 1024L * 1024L;
  cout << "Number of results: " << num_results << endl;
  cout << "Mem Limit: " << mem_limit << " bytes" << endl;
  cout << "Processsing input file " << file_path << "..." << endl;

  TopNFreqUrls instance(file_path, num_results, mem_limit);
  instance.Run();
  return 0;
}