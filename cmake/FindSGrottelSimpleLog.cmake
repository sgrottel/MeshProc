# FindSGrottelSimpleLog.cmake
#
# CMake find module for the header-only NuGet package
# SGrottel.SimpleLog.Cpp

if(NOT DEFINED NUGET_PACKAGES_DIR)
    set(NUGET_PACKAGES_DIR "${CMAKE_SOURCE_DIR}/packages")
endif()

file(GLOB _simplelog_candidates
    "${NUGET_PACKAGES_DIR}/SGrottel.SimpleLog.Cpp*/build/native/include"
)

if(NOT _simplelog_candidates)
    message(FATAL_ERROR "SimpleLog: No SGrottel.SimpleLog.Cpp package found under ${NUGET_PACKAGES_DIR}.")
endif()

# Locate the include directory restored by NuGet
find_path(SGrottelSimpleLog_INCLUDE_DIR
    NAMES SimpleLog/SimpleLog.hpp
    PATHS ${_simplelog_candidates}
    NO_DEFAULT_PATH
)

if(NOT SGrottelSimpleLog_INCLUDE_DIR)
    message(FATAL_ERROR
        "SimpleLog: Could not find SimpleLog/SimpleLog.hpp in: "
        "${_simplelog_candidates}"
    )
endif()

# Create an INTERFACE target (header-only)
add_library(SGrottelSimpleLog INTERFACE)

# Add include directory
target_include_directories(SGrottelSimpleLog INTERFACE
    ${SGrottelSimpleLog_INCLUDE_DIR}
)
