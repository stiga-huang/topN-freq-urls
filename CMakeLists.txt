cmake_minimum_required(VERSION 3.9)
project(topN_freq_urls)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)
add_subdirectory(test)

add_executable(topN_freq_urls src/main.cpp src/mem-pool.h src/mem-pool.cpp src/file-manager.cpp src/file-manager.h src/data.h src/topn-freq-urls.h src/topn-freq-urls.cpp)