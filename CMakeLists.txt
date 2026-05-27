# ==========================================
# 1. Meta Data & Project Config
# ==========================================
cmake_minimum_required(VERSION 3.10)
project(Four-Color VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# 2. External Dependencies (vcpkg)
# ==========================================
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)


# ==========================================
# 3. Define Targets (Your Libraries & Executables)
# ==========================================
# First, create your Point library (assuming Point.cpp/h are in your src folder)
add_library(PointLib src/Point.cpp src/Point.h)

# Next, create your main application executable
add_executable(my_executable src/main.cpp)

# ==========================================
# 4. Target Configuration & Linking
# ==========================================
# Tell CMake that anyone linking to PointLib needs to know its directory
target_include_directories(PointLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link everything to your main executable 
# (Notice we use 'my_executable' consistently here)
target_link_libraries(my_executable PRIVATE 
    glfw 
    glad::glad 
    PointLib
)