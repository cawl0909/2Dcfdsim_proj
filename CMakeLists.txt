cmake_minimum_required(VERSION 3.10.0)
project(cfd_sim VERSION 0.1.0 LANGUAGES C CXX)

#sdl3

find_package(SDL3 CONFIG REQUIRED)

# Imgui
include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG master
)

FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    #Backends
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp"
)

target_include_directories(imgui PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)

add_executable(${PROJECT_NAME}
src/main.cpp 
src/vectors.cpp
src/Fluid.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 imgui)

