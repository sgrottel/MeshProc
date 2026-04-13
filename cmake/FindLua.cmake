# FindLua.cmake
#
# CMake find module for the NuGet package
# Lua

if(NOT DEFINED NUGET_PACKAGES_DIR)
    set(NUGET_PACKAGES_DIR "${CMAKE_SOURCE_DIR}/packages")
endif()

file(GLOB _lua_candidates
    "${NUGET_PACKAGES_DIR}/lua*/build/native/include"
)

if(NOT _lua_candidates)
    message(FATAL_ERROR "Lua: No lua package found under ${NUGET_PACKAGES_DIR}.")
endif()

# Locate the include directory restored by NuGet
find_path(_lua_include_dir
    NAMES lua.hpp
    PATHS ${_lua_candidates}
    NO_DEFAULT_PATH
)

if(NOT _lua_include_dir)
    message(FATAL_ERROR
        "Lua: Could not find lua.hpp in: "
        "${_lua_candidates}"
    )
endif()

set(_lua_pkg_dir "${_lua_include_dir}/../../..")

# PlatformName (MSBuild style)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_lua_platform "x64")
else()
    set(_lua_platform "Win32")
endif()

# Toolset (your "LuaRuntimePlatform" concept, e.g. v143)
# You can set this in your top-level CMakeLists.txt:
#   set(LuaToolset "v143")
if(NOT DEFINED LuaToolset)
    # Try to infer from MSVC toolset if available
    if(MSVC_TOOLSET_VERSION)
        # MSVC_TOOLSET_VERSION is like 143; we want "v143"
        set(LuaToolset "v${MSVC_TOOLSET_VERSION}")
    else()
        message(FATAL_ERROR
            "Lua: LuaToolset is not set and MSVC_TOOLSET_VERSION is unknown. "
            "Set LuaToolset explicitly, e.g. set(LuaToolset \"v143\")."
        )
    endif()
endif()

# LuaConfiguration — Debug/Release
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(_lua_config "Debug")
    else()
        set(_lua_config "Release")
    endif()
else()
    # For multi-config (VS), we can't know at configure time which config
    # will be used, so we use generator expressions.
    set(_lua_config "$<IF:$<CONFIG:Debug>,Debug,Release>")
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
    # Single-config: concrete path
    set(_lua_lib_dir
        "${_lua_pkg_dir}/build/native/lib/${_lua_platform}/${LuaToolset}/${_lua_config}"
    )
    set(_lua_lib "${_lua_lib_dir}/lua_static.lib")

    if(NOT EXISTS "${_lua_lib}")
        message(FATAL_ERROR
            "Lua: static library not found:\n"
            "  ${_lua_lib}\n"
            "Expected structure:\n"
            "  lib/<PlatformName>/<Toolset>/<Configuration>/lua_static.lib"
        )
    endif()
else()
    # Multi-config: use generator expression for IMPORTED_LOCATION_<CONFIG>
    set(_lua_lib_debug
        "${_lua_pkg_dir}/build/native/lib/${_lua_platform}/${LuaToolset}/Debug/lua_static.lib"
    )
    set(_lua_lib_release
        "${_lua_pkg_dir}/build/native/lib/${_lua_platform}/${LuaToolset}/Release/lua_static.lib"
    )

    foreach(_p "${_lua_lib_debug}" "${_lua_lib_release}")
        if(NOT EXISTS "${_p}")
            message(FATAL_ERROR
                "Lua: expected multi-config library not found:\n  ${_p}"
            )
        endif()
    endforeach()
endif()

add_library(Lua::Lua STATIC IMPORTED GLOBAL)

if(NOT CMAKE_CONFIGURATION_TYPES)
    set_target_properties(Lua::Lua PROPERTIES
        IMPORTED_LOCATION "${_lua_lib}"
        INTERFACE_INCLUDE_DIRECTORIES "${_lua_include_dir}"
    )
else()
    set_target_properties(Lua::Lua PROPERTIES
        IMPORTED_LOCATION_DEBUG   "${_lua_lib_debug}"
        IMPORTED_LOCATION_RELEASE "${_lua_lib_release}"
        INTERFACE_INCLUDE_DIRECTORIES "${_lua_include_dir}"
    )
endif()

target_compile_definitions(Lua::Lua INTERFACE HAS_LUA)
