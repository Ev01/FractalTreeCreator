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
    src/main.cpp
    src/tree.cpp
    src/tree_thread.cpp
    src/ui.cpp
    src/debug.cpp
    src/render.cpp
    vendored/imgui/imgui.cpp
    vendored/imgui/imgui_demo.cpp
    vendored/imgui/imgui_draw.cpp
    vendored/imgui/imgui_impl_sdl3.cpp
    vendored/imgui/imgui_impl_sdlrenderer3.cpp
    vendored/imgui/imgui_impl_opengl3.cpp
    vendored/imgui/imgui_tables.cpp
    vendored/imgui/imgui_widgets.cpp
    src/glad/glad.c

)

target_link_libraries(fractal_tree PRIVATE SDL3::SDL3 -static-libgcc -static-libstdc++)

# CPack stuff

install(TARGETS fractal_tree RUNTIME_DEPENDENCY_SET deps 
    RUNTIME DESTINATION bin)
# Install dependencies but exclude system libraries
IF (WIN32)
    install(RUNTIME_DEPENDENCY_SET deps 
        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
        DIRECTORIES
            $<TARGET_FILE_DIR:SDL3::SDL3>
    )

    #install( CODE [[
    #    file(GET_RUNTIME_DEPENDENCIES
    #        EXECUTABLES
    #            $<TARGET_FILE:fractal_tree>
    #        RESOLVED_DEPENDENCIES_VAR _r_deps
    #        UNRESOLVED_DEPENDENCIES_VAR _u_deps
    #        DIRECTORIES
    #            $<TARGET_FILE_DIR:SDL3::SDL3>
    #    )
    #    foreach(_file ${_r_deps})
    #        string(TOLOWER ${_file} _file_lower)
    #        if(NOT ${_file_lower} MATCHES "c:[\\/]windows[\\/]system32.*")
    #            file(INSTALL
    #                DESTINATION "${CMAKE_INSTALL_PREFIX}"
    #                TYPE SHARED_LIBRARY
    #                FOLLOW_SYMLINK_CHAIN
    #                FILES "${_file}"
    #            )
    #        endif()
    #    endforeach()
    #    message("UNRESOLVED_DEPENDENCIES_VAR: ${_u_deps}")
    #]] )
ELSE()
    set(exclude_regexes ".*libc\.so.*|.*libm\.so.*|.*ld-linux.*")
    install(RUNTIME_DEPENDENCY_SET deps PRE_EXCLUDE_REGEXES ${exclude_regexes})
ENDIF()
# Put shaders in the bin directory
install(DIRECTORY shaders DESTINATION .)
# Install demo trees with package
install(DIRECTORY DemoTrees DESTINATION .)
# Enable packaging
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
IF (WIN32)
    set(CPACK_GENERATOR "ZIP")
ELSE ()
    set(CPACK_GENERATOR "TGZ")
ENDIF ()

include(CPack)
