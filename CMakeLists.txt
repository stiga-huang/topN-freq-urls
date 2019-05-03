cmake_minimum_required(VERSION 3.9)
project(topN_freq_urls)

set(CMAKE_CXX_STANDARD 11)

# Determine the build type. If no build build type is specified, default to debug builds
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif(NOT CMAKE_BUILD_TYPE)
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

# Debug information is stored as dwarf2 to be as compatible as possible
SET(CXX_FLAGS_DEBUG "${CXX_GCC_FLAGS} -ggdb -gdwarf-2 -Werror")
SET(CXX_FLAGS_RELEASE "${CXX_GCC_FLAGS} -O2 -gdwarf-2")
SET(CXX_FLAGS_ADDRESS_SANITIZER
        "${CXX_CLANG_FLAGS} -O1 -g -fsanitize=address -fno-omit-frame-pointer -DADDRESS_SANITIZER")

# Set compile flags based on the build type.
if ("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    SET(CMAKE_CXX_FLAGS ${CXX_FLAGS_DEBUG})
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    SET(CMAKE_CXX_FLAGS ${CXX_FLAGS_RELEASE})
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "ADDRESS_SANITIZER")
    SET(CMAKE_CXX_FLAGS "${CXX_FLAGS_ADDRESS_SANITIZER}")
else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

MESSAGE(STATUS "Compiler Flags: ${CMAKE_CXX_FLAGS}")

set (GTEST_VERSION "1.8.1")
set (GTEST_INCLUDE_DIRS
        "${CMAKE_SOURCE_DIR}/libs/googletest-${GMOCK_VERSION}/googletest/include"
)
set (GTEST_LIBRARIES gtest)
if(NOT APPLE AND NOT MSVC)
    list (APPEND GTEST_LIBRARIES pthread)
endif(NOT APPLE AND NOT MSVC)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(libs/googletest-${GTEST_VERSION})

add_executable(topN_freq_urls src/main.cpp src/mem-pool.h src/mem-pool.cpp src/file-manager.cpp src/file-manager.h src/data.h src/topn-freq-urls.h src/topn-freq-urls.cpp)
# link profiler if it's installed
#target_link_libraries(topN_freq_urls profiler)