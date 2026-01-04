cmake_minimum_required(VERSION 3.10.0)
project(cfd_sim VERSION 0.1.0 LANGUAGES C CXX)

find_package(SDL3 CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
src/main.cpp 
src/vectors.cpp
src/Fluid.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)