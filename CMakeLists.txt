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

option(BUILD_STATIC "Attempt to link libraries statically to keep runtime dependencies down" OFF)

if (BUILD_STATIC)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

    add_subdirectory(glfw)
endif()

add_library(imgui STATIC
    imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
    imgui/misc/freetype/imgui_freetype.cpp
    imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp
)
target_link_libraries(imgui PUBLIC glfw GL freetype)
target_include_directories(imgui PRIVATE /usr/include/freetype2)
target_include_directories(imgui PUBLIC imgui/ imgui/backends/)
target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE)

add_executable(EGGMouseConfig main.cpp
    application.hpp application.cpp
    device.hpp device.cpp)
target_compile_options(EGGMouseConfig PRIVATE -Wno-address-of-packed-member)
target_compile_options(EGGMouseConfig PUBLIC -fdata-sections -ffunction-sections)
target_compile_definitions(EGGMouseConfig PRIVATE PROJECT_VERSION="${GIT_TAG}" COMPILER_VERSION="${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}" PRODUCTID=0x1966)
target_link_options(EGGMouseConfig PUBLIC -Wl,--gc-sections)
target_link_libraries(EGGMouseConfig PRIVATE imgui)
if (BUILD_STATIC)
    target_link_libraries(EGGMouseConfig PRIVATE libhidapi-libusb.a libusb-1.0.a udev)
    target_link_options(EGGMouseConfig PUBLIC -static-libstdc++ -static-libgcc)
else()
    target_link_libraries(EGGMouseConfig PRIVATE hidapi-libusb freetype)
endif()

include(GNUInstallDirs)
install(TARGETS EGGMouseConfig
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
