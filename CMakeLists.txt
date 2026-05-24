# meta data
# 1. Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)
# 2. Set the project name and version
project(Four-Color VERSION 1.0 LANGUAGES CXX)

#project config
# 3. Specify the C++ standard to use
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#external dependencies
# Find the packages via vcpkg
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)


# 4. Define the executable and the source files needed to build it
add_executable(my_executable src/main.cpp)


# Link them to your program
target_link_libraries(my_executable PRIVATE glfw glad::glad)