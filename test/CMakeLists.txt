add_executable(tests Tests.cpp)
target_link_libraries(tests PRIVATE lob_lib gtest gtest_main pthread)
add_test(NAME AllTests COMMAND tests)
