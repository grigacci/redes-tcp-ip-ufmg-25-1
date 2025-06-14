cmake_minimum_required(VERSION 3.16)
project(simplex-talk C)

add_executable(client client.c)
add_executable(server server.c)

target_compile_options(client PRIVATE -w)
target_compile_options(server PRIVATE -w)