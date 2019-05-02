#!/bin/bash

: ${CMAKE_BUILD_TYPE:=Debug}

# parse command line options
for ARG in $*
do
  case "$ARG" in
      -release)
        CMAKE_BUILD_TYPE=Release
        ;;
      -asan)
        CMAKE_BUILD_TYPE=ADDRESS_SANITIZER
        ;;
    esac
  shift;
done

cmake . "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
make -j4
