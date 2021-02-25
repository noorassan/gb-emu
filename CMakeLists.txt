cmake_minimum_required(VERSION 3.19.5)
project(gb-emu)

# C++ 17
set(CMAKE_CXX_STANDARD 17)

# Bring in headers from include/
include_directories(include)

# Bring in source code
file(GLOB SOURCES "src/*.cc" "src/memory_bank_controllers/*.cc")

# SDL Graphics Library
find_package(SDL2 REQUIRED)
include_directories(SYSTEM ${SDL2_INCLUDE_DIRS})

add_executable(gb-emu ${SOURCES})
target_link_libraries(gb-emu ${SDL2_LIBRARIES})