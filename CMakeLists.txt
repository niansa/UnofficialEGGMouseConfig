cmake_minimum_required(VERSION 3.16)

project(EGGMouseConfig LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Git)

execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE GIT_RESULT
)
message(STATUS "Determined EGG Mouse Config version ${GIT_TAG}")

if (NOT EMSCRIPTEN)
    option(BUILD_STATIC "Attempt to link libraries statically to keep runtime dependencies down" OFF)
else()
    set(BUILD_STATIC OFF)
endif()

if (BUILD_STATIC)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

    add_subdirectory(glfw)
endif()

add_library(imgui STATIC
    imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
)
target_link_libraries(imgui PUBLIC GL)
target_include_directories(imgui PRIVATE /usr/include/freetype2)
target_include_directories(imgui PUBLIC imgui/ imgui/backends/)
if (NOT EMSCRIPTEN)
    # Glfw+GL3 support
    target_sources(imgui PRIVATE imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp)
    target_link_libraries(imgui PUBLIC glfw)

    # Freetype support
    target_sources(imgui PRIVATE imgui/misc/freetype/imgui_freetype.cpp)
    target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE)
    target_link_libraries(imgui PUBLIC freetype)
else()
    # SDL2+SDL_Renderer support
    target_sources(imgui PRIVATE imgui/backends/imgui_impl_sdl2.cpp imgui/backends/imgui_impl_sdlrenderer2.cpp)
    target_link_libraries(imgui PUBLIC SDL2)
    target_compile_options(imgui PUBLIC -sUSE_SDL=2)
endif()

add_executable(EGGMouseConfig
    application.hpp application.cpp
    device.hpp device.cpp
    config.hpp config.cpp
    themes.inc)
target_compile_options(EGGMouseConfig PRIVATE -Wno-address-of-packed-member)
target_compile_options(EGGMouseConfig PUBLIC -fdata-sections -ffunction-sections)
target_compile_definitions(EGGMouseConfig PRIVATE PROJECT_VERSION="${GIT_TAG}" COMPILER_VERSION="${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
target_link_options(EGGMouseConfig PUBLIC -Wl,--gc-sections)
target_link_libraries(EGGMouseConfig PRIVATE imgui)
if (NOT EMSCRIPTEN)
    target_sources(EGGMouseConfig PRIVATE main-glfw.cpp)
    if (BUILD_STATIC)
        target_link_libraries(EGGMouseConfig PUBLIC -static-libstdc++ -static-libgcc PRIVATE libhidapi-hidraw.a udev)
    else()
        target_link_libraries(EGGMouseConfig PRIVATE hidapi-hidraw)
    endif()
else()
    target_sources(EGGMouseConfig PRIVATE main-sdl2.cpp fake-hidapi-emscripten.hpp fake-hidapi-emscripten.cpp)
    target_link_options(EGGMouseConfig PUBLIC -sASYNCIFY)
endif()

include(GNUInstallDirs)
install(TARGETS EGGMouseConfig
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (NOT EMSCRIPTEN)
    if (CMAKE_BUILD_TYPE STREQUAL MinSizeRel)
        message(STATUS "Executable compression enabled")

        add_custom_command(
          OUTPUT EGGMouseConfigStripped
          COMMAND strip EGGMouseConfig -o EGGMouseConfigStripped
          DEPENDS EGGMouseConfig
          COMMENT "Stripping executable"
          VERBATIM
        )
        add_custom_command(
          OUTPUT EGGMouseConfigPacked
          COMMAND upx --best --ultra-brute EGGMouseConfigStripped -o EGGMouseConfigPacked
          DEPENDS EGGMouseConfigStripped
          COMMENT "Compressing executable"
          VERBATIM
        )
        add_custom_target(
          StripAndCompress ALL
          DEPENDS EGGMouseConfigPacked
        )
    endif()
endif()
