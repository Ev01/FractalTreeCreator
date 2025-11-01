cmake_minimum_required(VERSION 3.26)
project(fractal_tree C CXX)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

#set(CMAKE_EXE_LINKER_FLAGS " -static")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -static-libgcc -static-libstdc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# This assumes the SDL source is available in vendored/SDL
#add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

add_executable(fractal_tree)

target_sources(fractal_tree
PRIVATE
    main.cpp
    tree.cpp
    ui.cpp
    debug.cpp
    vendored/imgui/imgui.cpp
    vendored/imgui/imgui_demo.cpp
    vendored/imgui/imgui_draw.cpp
    vendored/imgui/imgui_impl_sdl3.cpp
    vendored/imgui/imgui_impl_sdlrenderer3.cpp
    vendored/imgui/imgui_tables.cpp
    vendored/imgui/imgui_widgets.cpp

)

target_link_libraries(fractal_tree PRIVATE SDL3::SDL3 -static-libgcc -static-libstdc++)
