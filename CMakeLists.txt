cmake_minimum_required(VERSION 3.10.0)
project(cfd_sim VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
src/main.cpp 
src/vectors.cpp
)

