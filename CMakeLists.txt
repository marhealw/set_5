cmake_minimum_required(VERSION 3.16)
project(hll_research LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(core
    src/stream/RandomStreamGen.cpp
    src/hash/HashFuncGen.cpp
)

target_include_directories(core PUBLIC src)
add_executable(smoke src/experiments/smoke.cpp)
target_link_libraries(smoke PRIVATE core)
