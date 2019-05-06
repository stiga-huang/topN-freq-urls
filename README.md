# topN-freq-urls

Count URLs and get the most frequent 100 urls. The input file is expected to be 100GB. Process memory is expected to be 1GB.
It takes ~40 minutes in using one disk in a Ubuntu machine.

## Generate test data
Prepare your expected answer, e.g. gen-data/my_top10_ans. Then use gen-data/gen_file_from_ans.py to generate input file:
```
./gen_file_from_ans.py ans_file file_size_in_gb target_dir
```
The file_size_in_gb can be float. For example, to generate a 100MB test file:
```
cd gen-data
./gen_file_from_ans.py my_top10_ans 0.1 .
```

## Build
This project can be built in MacOS or Ubuntu 14.04 with 64bit CPU.
```
./build.sh
```
The default build mode DEBUG. For release build, use
```
./build.sh -release
```
We also integrated with AddressSantinizer. To enable address santinizer build, use 
```
./build.sh -asan
```

## Usage
```
./topN_freq_urls file_path num_results mem_limit_in_mb
```

## Overview
The basic framework is an external sorting, which is divided into two stage:
### 1. Partial Sort and Pre-Aggregate
The first stage loads URLs into memory buffer (limited by the mem_limit), sort them and spill sorted URLs as a file.
For a 100GB input file and 1GB mem_limit, we'll generate 100GB/1GB = 100 spilled files. URLs in each file are sorted.
Pre-Aggregation is also performed in this stage. The format of each row in the spilled file is (URL, count) separated by '\t'.

### 2. Merge and Final-Aggregate
Merge all the spilled files and performed Final-Aggregation. A heap is used to merge the spill files.
So (URL, count) pairs with the same URLs can be popped in sequence. Then we can merge them to get the global count.

Finally, We maintain a TopN heap to store the recently TopN frequent URLs' pairs, i.e. (URL, count).

## TODO
There are still some opportunities for optimization. Here is a profile in a latest test:
```
    4879  25.5%  25.5%     4879  25.5% std::istream::get
    2781  14.5%  40.0%     2781  14.5% __write_nocancel
    1409   7.4%  47.4%     1409   7.4% __memcmp_sse4_1
     767   4.0%  51.4%     1021   5.3% __gnu_cxx::__normal_iterator::operator+
     759   4.0%  55.3%      759   4.0% topN_freq::StringVal::StrCompare
     596   3.1%  58.4%     1122   5.9% std::_Hashtable::_M_find_before_node
     468   2.4%  60.9%      468   2.4% __read_nocancel
     366   1.9%  62.8%      366   1.9% __memcpy_sse2_unaligned
     338   1.8%  64.6%      338   1.8% __gnu_cxx::__normal_iterator::__normal_iterator
     332   1.7%  66.3%      332   1.7% std::__detail::_Mod_range_hashing::operator
     326   1.7%  68.0%     1799   9.4% std::__adjust_heap
     302   1.6%  69.6%      302   1.6% __gnu_cxx::__normal_iterator::operator*
     287   1.5%  71.1%      287   1.5% malloc_consolidate
     279   1.5%  72.5%      779   4.1% std::__detail::_Hash_code_base::_M_bucket_index
     252   1.3%  73.9%      998   5.2% std::__push_heap
     199   1.0%  74.9%      199   1.0% std::equal_to::operator
     170   0.9%  75.8%      170   0.9% _int_malloc
     157   0.8%  76.6%      157   0.8% __GI___libc_free
     149   0.8%  77.4%      149   0.8% _int_free
     146   0.8%  78.1%      146   0.8% std::move
     115   0.6%  78.7%      185   1.0% topN_freq::ResultTuple::operator<
     105   0.5%  79.3%      784   4.1% std::_Hashtable::_M_erase
     103   0.5%  79.8%      103   0.5% __unlink
     102   0.5%  80.4%      131   0.7% std::__detail::_Hash_code_base::_M_h2
      99   0.5%  80.9%       99   0.5% std::istream::sentry::sentry
      98   0.5%  81.4%       98   0.5% std::__detail::_Hashtable_ebo_helper::_S_cget
      97   0.5%  81.9%       97   0.5% std::forward
      92   0.5%  82.4%     1452   7.6% std::__unguarded_partition
      80   0.4%  82.8%     1950  10.2% topN_freq::MemPool::Free
      78   0.4%  83.2%      485   2.5% topN_freq::StringLess::operator
      77   0.4%  83.6%       77   0.4% __gnu_cxx::__normal_iterator::operator--
      77   0.4%  84.0%     1484   7.8% topN_freq::StringVal::operator<
      76   0.4%  84.4%     1576   8.2% std::__detail::_Map_base::operator[]
      73   0.4%  84.8%       80   0.4% __gnu_cxx::operator-
      69   0.4%  85.1%       69   0.4% __gnu_cxx::__normal_iterator::operator++
      69   0.4%  85.5%      931   4.9% std::_Hashtable::_M_bucket_index
      63   0.3%  85.8%     1041   5.4% std::_Hashtable::_M_find_node
      59   0.3%  86.1%       85   0.4% std::__detail::_Hash_code_base::_M_h1
      57   0.3%  86.4%       57   0.3% std::num_get::_M_extract_int
      57   0.3%  86.7%      186   1.0% std::vector::push_back
      56   0.3%  87.0%      113   0.6% topN_freq::FileManager::ReadLine@405694
      52   0.3%  87.3%      114   0.6% std::get
      52   0.3%  87.6%      224   1.2% std::less::operator
      52   0.3%  87.8%       81   0.4% std::vector::begin
      51   0.3%  88.1%       63   0.3% __gnu_cxx::operator< 
      51   0.3%  88.4%      189   1.0% std::_Hashtable::_M_insert_bucket_begin
      49   0.3%  88.6%       49   0.3% __gnu_cxx::__normal_iterator::base
      49   0.3%  88.9%     2389  12.5% topN_freq::FileMerger::NextResultTuple
      48   0.3%  89.1%      117   0.6% std::__detail::_Hash_code_base::_M_hash_code
      43   0.2%  89.4%      121   0.6% std::__detail::_Select1st::operator
      42   0.2%  89.6%       42   0.2% std::__detail::_Node_iterator_base::_Node_iterator_base
      36   0.2%  89.8%      358   1.9% std::__unguarded_linear_insert
      35   0.2%  90.0%       43   0.2% _ZN9__gnu_cxx13new_allocatorIN9topN_freq9StringValEE9constructIS2_IRKS2_EEEvPT_DpOT0_
      35   0.2%  90.1%       73   0.4% std::__detail::_Node_iterator::_Node_iterator
      34   0.2%  90.3%      231   1.2% _ZN9__gnu_cxx13new_allocatorINSt8__detail10_Hash_nodeISt4pairIKPcmELb0EEEE9constructIS7_IRKSt21piecewise_construct_tSt5tupleIIRS5_EESD_IIEEEEEvPT_DpOT0_
      34   0.2%  90.5%      311   1.6% std::_Hashtable::_M_allocate_node
      34   0.2%  90.7%      291   1.5% std::__detail::_Equal_helper::_S_equals
```
Possible optimization points:

 - Refactor the MemPool. Load 1GB URLs direcrly into the memory, instead of loading URLs line by line. After sort and spill, free the MemPool at once, instead of freeing URLs one by one. This can optimize time spent by std::istream::get. The hash map operations (e.g. std::\_Hashtable::\_M_find_before_node) of the current MemPool can also be saved.
 - Don't spill the last file. Aggregate it in memory with other spilled files. This can save 1% IO (since we have 100 spilled files).
 - Try mmap in reading files.
 - Spill to different disks. For example, if we can leverage 4 disks, divide the memory pool into 4 bucket. Sort URLs in each bucket in parallel and then spill buckets into different disks in parallel. This can also boost the Final-Aggregation stage since we have 4 times read speed than before.
