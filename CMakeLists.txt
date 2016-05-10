cmake_minimum_required(VERSION 2.6)
project(ginkgo CXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(Boost 1.55.0 COMPONENTS program_options iostreams system filesystem REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(JsonCpp REQUIRED)

if(BUILD_GTK3_GUI_TOOLS)
	pkg_check_modules(GTKmm REQUIRED gtkmm-3.0>=3.12.0)
endif(BUILD_GTK3_GUI_TOOLS)

option(BUILD_GTK3_GUI_TOOLS "Build GTK3-based GUI tools to render benchmark results" ON)
option(BUILD_TESTS "Build unit tests" OFF)

set(CMAKE_CXX_FLAGS "-Wall -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
add_definitions(-std=c++14)

add_subdirectory(src)
add_subdirectory(apps)
if(BUILD_TESTS)
	add_subdirectory(tests)
endif(BUILD_TESTS)
