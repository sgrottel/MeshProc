#
# MeshProc
# Simple Triangle Mesh Processor
# Copyright by SGrottel
#
cmake_minimum_required(VERSION 3.20)

# version information
set(MESHPROC_VERSION_MAJOR 0)
set(MESHPROC_VERSION_MINOR 6)
set(MESHPROC_VERSION_PATCH 2)
set(MESHPROC_VERSION_YEARSTR "2025-2026")
# build number to be set by CI
set(MESHPROC_VERSION_BUILD 0 CACHE STRING "Build number")

# CMake project
project(MeshProc
    VERSION "${MESHPROC_VERSION_MAJOR}.${MESHPROC_VERSION_MINOR}.${MESHPROC_VERSION_PATCH}.${MESHPROC_VERSION_BUILD}"
    DESCRIPTION "SGrottel's Mesh Processor"
    LANGUAGES CXX
)

# cpp 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(/Zc:__cplusplus)

# nuget
set(NUGET_PACKAGES_DIR "${CMAKE_SOURCE_DIR}/packages")

# vcpkg
set(VCPKG_TARGET_TRIPLET "x64-windows")
list(APPEND CMAKE_PREFIX_PATH
    "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}"
)

# custom find module scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(src)
