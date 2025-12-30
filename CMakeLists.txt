cmake_minimum_required(VERSION 3.10)
project(MyProject)

# SFML 3 requires C++17 or newer
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find SFML 3 (Notice we don't force version 2.5 anymore)
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

# 2. Add your executable
add_executable(my_program main.cpp)

# 3. Link using the new SFML 3 target naming convention (SFML::Component)
target_link_libraries(my_program SFML::Graphics SFML::Window SFML::System)