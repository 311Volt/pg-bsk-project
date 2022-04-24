cmake_minimum_required(VERSION 3.4.0)
project(pg-bsk-project VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "-std=c++17")

file(GLOB_RECURSE portable8439_sources "portable8439/src/*.c")
file(GLOB_RECURSE app_sources "src/*.cpp" "src/*.c")
add_library(portable8439_bsk STATIC ${portable8439_sources})

include("secp256k1_bsk.cmake")

add_subdirectory("rpclib")

add_executable(pg-bsk-project ${app_sources})

target_link_libraries(pg-bsk-project PUBLIC secp256k1_bsk)
target_link_libraries(pg-bsk-project PUBLIC portable8439_bsk)
target_link_libraries(pg-bsk-project PUBLIC rpc)


target_include_directories(pg-bsk-project PUBLIC "src")
target_include_directories(pg-bsk-project PUBLIC "portable8439/src")
target_include_directories(pg-bsk-project PUBLIC "secp256k1/include")
target_include_directories(pg-bsk-project PUBLIC "rpclib/include")
target_include_directories(pg-bsk-project PUBLIC "dpp")




