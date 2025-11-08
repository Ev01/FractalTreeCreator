cmake_minimum_required(VERSION 3.26)
project(fractal_tree LANGUAGES C CXX VERSION 0.1.0)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

#set(CMAKE_EXE_LINKER_FLAGS " -static")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -static-libgcc -static-libstdc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

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
    render.cpp
    vendored/imgui/imgui.cpp
    vendored/imgui/imgui_demo.cpp
    vendored/imgui/imgui_draw.cpp
    vendored/imgui/imgui_impl_sdl3.cpp
    vendored/imgui/imgui_impl_sdlrenderer3.cpp
    vendored/imgui/imgui_impl_opengl3.cpp
    vendored/imgui/imgui_tables.cpp
    vendored/imgui/imgui_widgets.cpp
    glad/glad.c

)

target_link_libraries(fractal_tree PRIVATE SDL3::SDL3 -static-libgcc -static-libstdc++)

# CPack stuff

install(TARGETS fractal_tree RUNTIME_DEPENDENCY_SET deps 
    RUNTIME DESTINATION bin)
# Install dependencies but exclude system libraries
install(RUNTIME_DEPENDENCY_SET deps PRE_EXCLUDE_REGEXES ".*libc\.so.*|.*libm\.so.*|.*ld-linux.*")
# Put shaders in the bin directory
install(DIRECTORY shaders DESTINATION .)
# Install demo trees with package
install(DIRECTORY DemoTrees DESTINATION .)
# Enable packaging
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_GENERATOR "TGZ")

include(CPack)
