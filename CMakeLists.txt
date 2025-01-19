cmake_minimum_required(VERSION 3.16)

project(EGGMouseConfig LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
target_link_libraries(imgui PUBLIC glfw GL)
if (BUILD_STATIC)
    target_link_libraries(imgui PRIVATE libfreetype.a libpng16.a libz.a libbz2.a libbrotlidec.a libbrotlicommon.a)
else()
    target_link_libraries(imgui PRIVATE freetype)
endif()
target_include_directories(imgui PRIVATE /usr/include/freetype2)
target_include_directories(imgui PUBLIC imgui/ imgui/backends/)
target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE)

add_executable(EGGMouseConfig main.cpp
    application.hpp application.cpp
    device.hpp device.cpp)
target_compile_options(EGGMouseConfig PRIVATE -Wno-address-of-packed-member)
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
