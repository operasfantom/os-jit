cmake_minimum_required(VERSION 3.12)
project(os-jit)

set(CMAKE_CXX_STANDARD 17)


add_library(rd_jit jit.cpp jit.h)
target_compile_features(rd_jit PUBLIC cxx_std_17)

add_executable(main main.cpp)
target_link_libraries(main rd_jit)

#add_executable(dlopen dlopen.cpp)
#target_link_libraries(dlopen)