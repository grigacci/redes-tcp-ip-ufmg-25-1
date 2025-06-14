cmake_minimum_required(VERSION 3.16)
project(simplex-talk C)

add_library(timer_lib timer/timer.c)

target_include_directories(timer_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/timer)

target_link_libraries(timer_lib PUBLIC rt)

add_executable(client client/client.c)
target_link_libraries(client PRIVATE timer_lib)

add_executable(server server/server.c)
target_link_libraries(server PRIVATE timer_lib)


target_compile_options(client PRIVATE -w)
target_compile_options(server PRIVATE -w)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -pedantic)