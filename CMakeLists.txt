cmake_minimum_required(VERSION 3.13)
project(TCP)

set(CMAKE_CXX_STANDARD 17)
add_executable(TCPClient client.c)
add_executable(TCPServer server.c minizip/zip.c minizip/minizip.c minizip/ioapi.c)
target_link_libraries(TCPServer z pthread)