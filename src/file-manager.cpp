#include <string>
#include <cstring>
#include <iostream>
#include "file-manager.h"

using namespace std;

namespace topN_freq {

bool FileManager::ReadLine(istream& in, unsigned int max_len, char* buffer,
    size_t* str_len, char delimiter) {
  if (in.get(buffer, max_len, delimiter)) {
    *str_len = in.gcount();
    in.get(); // read the tailing delimiter
    return true;
  }
  return false;
}

string FileManager::OutputFileName(const char* base_file_name, int file_index) {
  string output_file_name(base_file_name);
  output_file_name += ".out.";
  output_file_name += to_string(file_index);
  return output_file_name;
}

FileManager::~FileManager() {
//  in_.close();
//  out_.close();
}

bool FileManager::Open() {
  in_.open(file_name_, ifstream::in);
  if (in_.fail()) {
    cerr << "Failed to open " << file_name_ << ": " << strerror(errno) << endl;
    return false;
  }
  return true;
}

bool FileManager::Close() {
  in_.close();
  out_.close();
  return true;
}

bool FileManager::ReadLine(unsigned int max_len, char* buffer, size_t* str_len) {
  return ReadLine(in_, max_len, buffer, str_len);
}

bool FileManager::NewOutputFile() {
  out_.close();
  out_.open(OutputFileName(file_name_, output_files_++), ofstream::out);
  if (out_.fail()) {
    cerr << "Failed to open " << file_name_ << ": " << strerror(errno) << endl;
    return false;
  }
  return true;
}

bool FileManager::WriteResultTuple(ResultTuple& res) {
  out_.write(res.str.ptr, res.str.len);
  out_ << '\t' << res.cnt << endl;
  if (out_.fail()) {
    cerr << "Failed write middle results: " << strerror(errno) << endl;
    return false;
  }
  return true;
}

bool FileMerger::NextResultTupleInFile(std::istream* in, MemPool *pool, ResultTuple* res) {
  char url_buffer[MAX_URL_LEN];
  if (!FileManager::ReadLine(*in, MAX_URL_LEN, url_buffer, &res->str.len, '\t')) {
    return false;
  }
  res->str.ptr = pool->Allocate(res->str.len);
  memcpy(res->str.ptr, url_buffer, res->str.len);
  *in >> res->cnt;
  in->get(); // consume the tailing '\n'
  return true;
}

bool FileMerger::Open(int num_files) {
  drained_.resize(num_files, false);
  if (!test_mode) OpenInternal(num_files);
  // init the min heap
  for (int i = 0; i < num_files; ++i) {
    if (drained_[i]) continue;
    ResultTuple tuple;
    if (!NextResultTupleInFile(sorted_files_[i], mem_pool_, &tuple)) {
      drained_[i] = true;
      continue;
    }
    tuple.file_index = i;
    res_tuples_heap_.push(tuple);
  }
  return true;
}

bool FileMerger::OpenInternal(int num_files) {
  for (int i = 0; i < num_files; ++i) {
    sorted_files_.push_back(new ifstream(
        FileManager::OutputFileName(base_file_name_, i), ifstream::in));
    drained_[i] = false;
  }
  return true;
}

bool FileMerger::Close() {
  // in unit tests, we don't open any files so just return.
  if (test_mode) return true;
  for (istream* s : sorted_files_) {
    reinterpret_cast<ifstream*&>(s)->close();
    delete s;
  }
  int num_files = sorted_files_.size();
  for (int i = 0; i < num_files; ++i) {
    string file_name = FileManager::OutputFileName(base_file_name_, i);
    remove(file_name.c_str());
  }
  return true;
}

bool FileMerger::NextResultTuple(ResultTuple* res) {
  if (res_tuples_heap_.empty()) return false;
  *res = res_tuples_heap_.top();
  res_tuples_heap_.pop();
  int i = res->file_index;
  if (!drained_[i]) {
    ResultTuple tuple;
    if (!NextResultTupleInFile(sorted_files_[i], mem_pool_, &tuple)) {
      drained_[i] = true;
    } else {
      tuple.file_index = i;
      res_tuples_heap_.push(tuple);
    }
  }
  return true;
}
}
