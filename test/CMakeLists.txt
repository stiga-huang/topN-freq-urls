enable_testing()

include_directories(
        ${PROJECT_SOURCE_DIR}/src
        ${GTEST_INCLUDE_DIRS}
)

add_executable(topN_freq_urls_test
        mem-pool-test.cpp
        data-test.cpp
        file-manager-test.cpp)
#add_library(gtest ../libs/googletest-1.8.1/googletest/src/gtest-all.cc)
target_link_libraries(topN_freq_urls_test ${GTEST_LIBRARIES} mem-pool file-manager)

add_test(NAME topN_freq_urls_test COMMAND topN_freq_urls_test)
