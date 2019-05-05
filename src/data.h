#ifndef TOPN_FREQ_URLS_DATA_H
#define TOPN_FREQ_URLS_DATA_H

#include <cstdint>
#include <string>
#include <cstring>

namespace topN_freq {

struct StringVal {
  char* ptr;
  size_t len;

  bool operator<(const StringVal& other) const {
    return StrCompare(other) < 0;
  }

  bool StrEquals(const StringVal& other) const {
    return StrCompare(other) == 0;
  }

  /// Compare with other StringVal. Return 0 for equals. Return negative value
  /// if 'other' is larger. Return positive value if 'other' is smaller.
  inline int StrCompare(const StringVal& other) const {
    int min_len = len <= other.len ? len : other.len;
    int cmp = memcmp(ptr, other.ptr, min_len);
    if (cmp == 0) return (int)len - (int)other.len;
    return cmp;
  }

  std::string toString() {
    return std::string(ptr, len);
  }
};

struct ResultTuple {
  StringVal str;
  int cnt;
  /// The index of the sorted file this tuple come from. Used in merge stage.
  int file_index;

  /// Operator used in the heap (priority queue). The worst one will be on the top.
  /// So less counts or larger strings is treated as worst(larger).
  bool operator<(const ResultTuple& other) const{
    if (cnt > other.cnt) return true;
    if (cnt < other.cnt) return false;
    return str < other.str;
  }

  bool StrEquals(const ResultTuple& other) const {
    return str.StrEquals(other.str);
  }

  void Init(StringVal& str_val) {
    str = str_val;
    cnt = 1;
  }
  void SetEmpty() { cnt = 0; }
  bool IsEmpty() { return cnt == 0; }
  std::string toString() {
    return str.toString() + '\t' + std::to_string(cnt);
  }
};

/// Compare function used in merge sort stage. The URLs in spilled files are sorted
/// by string ascendingly. So we should still use the string order to merge. A min heap
/// is used in merging spilled files. So the smallest string will be on the heap top.
struct StringLess {
  /// Return false if 'a' should be placed on top of 'b' in the priority queue (i.e. a>b)
  bool operator()(const ResultTuple& a, const ResultTuple& b) {
    int cmp = a.str.StrCompare(b.str);
    if (cmp == 0) return a.cnt > b.cnt;
    return cmp > 0;
  }
};
}

#endif //TOPN_FREQ_URLS_DATA_H
