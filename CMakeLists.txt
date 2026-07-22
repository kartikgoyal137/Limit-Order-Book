cmake_minimum_required(VERSION 3.16)
project(LimitOrderBook VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native")

# Core library
add_library(lob_lib STATIC
    src/OrderBook.cpp
    src/MatchingEngine.cpp
)
target_include_directories(lob_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE lob_lib pthread)

# Tests
enable_testing()
add_subdirectory(googletest)
add_subdirectory(test)
