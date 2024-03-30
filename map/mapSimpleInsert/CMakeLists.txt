cmake_minimum_required (VERSION 3.8)
project (mapSimpleInsertCmake)

#include_directories ("${PROJECT_SOURCE_DIR}/dfglib/")

include_directories (
	"${PROJECT_SOURCE_DIR}/dfglib/"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Prevents 'decaying' to earlier standard if requested standard is not available
set(CMAKE_CXX_EXTENSIONS OFF)       # Prevents using e.g. -std=gnu++17 instead of -std=c++17
set(CMAKE_BUILD_TYPE "Release")

set(SOURCE
    mapSimpleInsert.cpp
)

add_executable(mapSimpleInsertCmake ${SOURCE})
