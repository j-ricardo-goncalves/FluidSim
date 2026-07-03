cmake_minimum_required(VERSION 3.16)
project(fluid_sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")

find_package(SDL2 REQUIRED)

add_executable(fluid_sim
    src/main.cpp
    src/fluid_grid.cpp
    src/renderer.cpp
)

target_include_directories(fluid_sim PRIVATE
    src
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(fluid_sim PRIVATE
    ${SDL2_LIBRARIES}
)
