#include <string>
#include <cstring>
#include <iostream>
#include "file-manager.h"

using namespace std;

namespace topN_freq {

bool FileManager::ReadLine(istream& in, unsigned int max_len, char* buffer,
    size_t* str_len, char delimiter) {
  *str_len = 0;
  char c;
  while (in.get(c)) {
    if (c == delimiter || c == '\0') {
      if (*str_len > 0) return true;
      continue; // skip empty lines and continue to read a URL
    }
    buffer[(*str_len)++] = c;
    if (*str_len == max_len) return true;
  }
  return false; // get here only at end of file
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
  return true;
}

void FileManager::WriteResultTuple(ResultTuple& res) {
  //std::cout << string(res.str.ptr, res.str.len) << endl;
  out_.write(res.str.ptr, res.str.len);
  out_ << '\t' << res.cnt << endl;
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
  drained_.resize(num_files);
  for (int i = 0; i < num_files; ++i) {
    sorted_files_.push_back(new ifstream(
        FileManager::OutputFileName(base_file_name_, i), ifstream::in));
    drained_[i] = false;
  }
  return true;
}

bool FileMerger::Close() {
  if (!test_mode) {
    for (istream* s : sorted_files_) {
      reinterpret_cast<ifstream*&>(s)->close();
      delete s;
    }
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
