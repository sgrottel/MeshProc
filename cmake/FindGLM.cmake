# FindGLM.cmake
#
# CMake find module for the header-only NuGet package
# GLM

if(NOT DEFINED NUGET_PACKAGES_DIR)
    set(NUGET_PACKAGES_DIR "${CMAKE_SOURCE_DIR}/packages")
endif()

file(GLOB _glm_candidates
    "${NUGET_PACKAGES_DIR}/glm*/build/native/include"
)

if(NOT _glm_candidates)
    message(FATAL_ERROR "GLM: No GLM package found under ${NUGET_PACKAGES_DIR}.")
endif()

# Locate the include directory restored by NuGet
find_path(GLM_INCLUDE_DIR
    NAMES glm/glm.hpp
    PATHS ${_glm_candidates}
    NO_DEFAULT_PATH
)

if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR
        "GLM: Could not find glm/glm.hpp in: "
        "${_glm_candidates}"
    )
endif()

# Create an INTERFACE target (header-only)
add_library(GLM INTERFACE)

# Add include directory
target_include_directories(GLM INTERFACE
    ${GLM_INCLUDE_DIR}
)
