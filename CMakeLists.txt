cmake_minimum_required(VERSION 3.28)
project(std)

set(CMAKE_CXX_STANDARD 20)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(SN_STD_BUILD_TESTS "Build tests" OFF)
if(SN_STD_BUILD_TESTS)
  enable_testing()
endif()

add_subdirectory(src)