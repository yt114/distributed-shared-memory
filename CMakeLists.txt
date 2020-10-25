cmake_minimum_required(VERSION 3.17)
project(distributed_shared_memory)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
list(APPEND CMAKE_PREFIX_PATH "/home/ye/.local/lib/cmake")
set(protobuf_MODULE_COMPATIBLE TRUE)

find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

add_executable(Client client.cpp client.h)
target_link_libraries(Client gRPC::grpc++)
