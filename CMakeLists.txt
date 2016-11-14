cmake_minimum_required(VERSION 2.6)
project(ginkgo CXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

option(GINKGO_BUILD_GTK3_GUI_TOOLS "Build GTK3-based GUI tools to render benchmark results" OFF)
option(GINKGO_BUILD_TESTS "Build unit tests" OFF)

find_package(Boost 1.55.0 COMPONENTS program_options iostreams system filesystem REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(JsonCpp REQUIRED)

if(GINKGO_BUILD_GTK3_GUI_TOOLS)
	pkg_check_modules(GTKmm REQUIRED gtkmm-3.0>=3.12.0)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CLASP_BUILD_APP OFF CACHE BOOL "whether or not to build the clasp application")
set(CLASP_BUILD_TEST OFF CACHE BOOL "whether or not to build clasp unit tests (requires CppUnit)")
set(CLINGO_ENABLE_LUA OFF CACHE BOOL "enable lua support")
set(CLINGO_ENABLE_SHARED OFF CACHE BOOL "build clingo library shared")
set(CLINGO_ENABLE_PYTHON OFF CACHE BOOL "enable python support")
set(LIB_POTASSCO_BUILD_TESTS OFF CACHE BOOL "whether or not to build tests")
set(LIB_POTASSCO_BUILD_APP OFF CACHE BOOL "whether or not to build lpconvert tool")

add_subdirectory(lib/clingo)
add_subdirectory(src)
add_subdirectory(apps)

if(GINKGO_BUILD_TESTS)
	add_subdirectory(tests)
endif()
