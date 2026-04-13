# FindSGrottelYaclap.cmake
#
# CMake find module for the header-only NuGet package
# SGrottel.yaclap

if(NOT DEFINED NUGET_PACKAGES_DIR)
    set(NUGET_PACKAGES_DIR "${CMAKE_SOURCE_DIR}/packages")
endif()

file(GLOB _yaclap_candidates
    "${NUGET_PACKAGES_DIR}/SGrottel.yaclap*/build/native/include"
)

if(NOT _yaclap_candidates)
    message(FATAL_ERROR "Yaclap: No SGrottel.yaclap package found under ${NUGET_PACKAGES_DIR}.")
endif()

# Locate the include directory restored by NuGet
find_path(SGrottelYaclap_INCLUDE_DIR
    NAMES yaclap.hpp
    PATHS ${_yaclap_candidates}
    NO_DEFAULT_PATH
)

if(NOT SGrottelYaclap_INCLUDE_DIR)
    message(FATAL_ERROR
        "Yaclap: Could not find yaclap.hpp in: "
        "${_yaclap_candidates}"
    )
endif()

# Create an INTERFACE target (header-only)
add_library(SGrottelYaclap INTERFACE)

# Add include directory
target_include_directories(SGrottelYaclap INTERFACE
    ${SGrottelYaclap_INCLUDE_DIR}
)
