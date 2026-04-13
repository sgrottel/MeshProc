#
# MeshProc
# Simple Triangle Mesh Processor
# Copyright by SGrottel
#
cmake_minimum_required(VERSION 3.20)

# version information
set(MESHPROC_VERSION_MAJOR 0)
set(MESHPROC_VERSION_MINOR 6)
set(MESHPROC_VERSION_PATCH 0)
set(MESHPROC_VERSION_YEARSTR "2025-2026")
# build number to be set by CI
set(MESHPROC_VERSION_BUILD 0 CACHE STRING "Build number")

# CMake project
project(MeshProc
    VERSION "${MESHPROC_VERSION_MAJOR}.${MESHPROC_VERSION_MINOR}.${MESHPROC_VERSION_PATCH}.${MESHPROC_VERSION_BUILD}"
    DESCRIPTION "SGrottel's Mesh Processor"
    LANGUAGES CXX
)

# restore nuget
set(NUGET_PACKAGES_DIR "${CMAKE_SOURCE_DIR}/packages")
## file(GLOB OLD_SIMPLELOG_DIRS
##     "${NUGET_PACKAGES_DIR}/SGrottel.SimpleLog.Cpp*"
## )
## foreach(dir ${OLD_SIMPLELOG_DIRS})
##     file(REMOVE_RECURSE "${dir}")
## endforeach()
execute_process(
    COMMAND nuget restore "${CMAKE_SOURCE_DIR}/packages.config"
            -PackagesDirectory "${NUGET_PACKAGES_DIR}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE NUGET_RESTORE_RESULT
)
if(NUGET_RESTORE_RESULT)
    message(FATAL_ERROR "NuGet restore failed")
endif()

# custom find module scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(src)
