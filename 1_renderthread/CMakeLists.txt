cmake_minimum_required(VERSION 3.28)
set(CMAKE_C_STANDARD 11)

project(lookupcolor)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(include)

file(GLOB FILES "src/*.c")
add_executable(game ${FILES})
target_link_libraries(game ${SDL2_LIBRARIES})
target_link_libraries(game m)
