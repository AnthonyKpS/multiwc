cmake_minimum_required(VERSION 3.17)
project(multiwc C)

set(CMAKE_C_STANDARD 11)

add_executable(multiwc main.c src/multiwc.c include/multiwc.h)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(multiwc Threads::Threads)