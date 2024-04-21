cmake_minimum_required (VERSION 3.8)
project (mapSimpleInsertCmake)

# Line below can be used to set clang++ as compiler e.g. on Ubuntu
#SET(CMAKE_CXX_COMPILER    "/usr/bin/clang++")

# Lines below can be used to set libc++ as standard library e.g. on Ubuntu
#     May need to install these first: sudo apt install libc++-dev libc++abi-dev
#add_compile_options("-stdlib=libc++")
#     https://stackoverflow.com/questions/51904938/linker-error-with-cmake-and-libcxx
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")


include_directories (
	"${PROJECT_SOURCE_DIR}/dfglib/"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Prevents 'decaying' to earlier standard if requested standard is not available
set(CMAKE_CXX_EXTENSIONS OFF)       # Prevents using e.g. -std=gnu++17 instead of -std=c++17
set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_VERBOSE_MAKEFILE ON)      # Sets more verbose compiler output during build

set(SOURCE
    mapSimpleInsert.cpp
)

add_executable(mapSimpleInsertCmake ${SOURCE})
