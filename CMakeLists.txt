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

set(GTEST_VERSION "1.8.1")
set(GTEST_INCLUDE_DIRS
        "${CMAKE_SOURCE_DIR}/libs/googletest-${GMOCK_VERSION}/googletest/include"
)
set(GTEST_LIBRARIES gtest)
if(NOT APPLE AND NOT MSVC)
    list(APPEND GTEST_LIBRARIES pthread)
endif(NOT APPLE AND NOT MSVC)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(libs/googletest-${GTEST_VERSION})

# Useful function copied from kudu
function(ADD_THIRDPARTY_LIB LIB_NAME)
  set(options)
  set(one_value_args SHARED_LIB STATIC_LIB)
  set(multi_value_args DEPS)
  cmake_parse_arguments(ARG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})
  if(ARG_UNPARSED_ARGUMENTS)
    message(SEND_ERROR "Error: unrecognized arguments: ${ARG_UNPARSED_ARGUMENTS}")
  endif()

  if(("${KUDU_LINK}" STREQUAL "s" AND ARG_STATIC_LIB) OR (NOT ARG_SHARED_LIB))
    if(NOT ARG_STATIC_LIB)
      message(FATAL_ERROR "No static or shared library provided for ${LIB_NAME}")
    endif()
    add_library(${LIB_NAME} STATIC IMPORTED)
    set_target_properties(${LIB_NAME}
      PROPERTIES IMPORTED_LOCATION "${ARG_STATIC_LIB}")
    message(STATUS "Added static library dependency ${LIB_NAME}: ${ARG_STATIC_LIB}")
  else()
    add_library(${LIB_NAME} SHARED IMPORTED)
    set_target_properties(${LIB_NAME}
      PROPERTIES IMPORTED_LOCATION "${ARG_SHARED_LIB}")
    message(STATUS "Added shared library dependency ${LIB_NAME}: ${ARG_SHARED_LIB}")
  endif()

  if(ARG_DEPS)
    set_target_properties(${LIB_NAME}
      PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES "${ARG_DEPS}")
  endif()

  # Set up an "exported variant" for this thirdparty library (see "Visibility"
  # above). It's the same as the real target, just with an "_exported" suffix.
  # We prefer the static archive if it exists (as it's akin to an "internal"
  # library), but we'll settle for the shared object if we must.
  #
  # A shared object exported variant will force any "leaf" library that
  # transitively depends on it to also depend on it at runtime; this is
  # desirable for some libraries (e.g. cyrus_sasl).
  set(LIB_NAME_EXPORTED ${LIB_NAME}_exported)
  if(ARG_STATIC_LIB)
    add_library(${LIB_NAME_EXPORTED} STATIC IMPORTED)
    set_target_properties(${LIB_NAME_EXPORTED}
      PROPERTIES IMPORTED_LOCATION "${ARG_STATIC_LIB}")
  else()
    add_library(${LIB_NAME_EXPORTED} SHARED IMPORTED)
    set_target_properties(${LIB_NAME_EXPORTED}
      PROPERTIES IMPORTED_LOCATION "${ARG_SHARED_LIB}")
  endif()
  if(ARG_DEPS)
    set_target_properties(${LIB_NAME_EXPORTED}
      PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES "${ARG_DEPS}")
  endif()
endfunction()

add_executable(topN_freq_urls src/main.cpp src/mem-pool.h src/mem-pool.cpp src/file-manager.cpp src/file-manager.h src/data.h src/topn-freq-urls.h src/topn-freq-urls.cpp)
if(APPLE)
    # link pprof if it's installed. Install it by `brew install google-perftools`
    # TODO: add test to detect whether pprof is installed.
    target_link_libraries(topN_freq_urls profiler)
else()
    set(GPERFTOOLS_ROOT "${CMAKE_SOURCE_DIR}/libs/gperftools-2.5")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")
    find_package(PProf REQUIRED)
    include_directories(SYSTEM ${PPROF_INCLUDE_DIR})
    ADD_THIRDPARTY_LIB(profiler STATIC_LIB ${PPROF_STATIC_LIB})
    target_link_libraries(topN_freq_urls profiler)
endif(APPLE)
