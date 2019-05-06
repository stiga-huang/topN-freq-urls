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
