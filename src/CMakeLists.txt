
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/VersionInfo.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/VersionInfo.h
    @ONLY
)

# Create the executable target
add_executable(MeshProc
    # main
    MeshProc.cpp
    # cmdline
    CmdLineArgs.cpp
    CmdLineArgs.h
    # version info
    VersionInfo.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/VersionInfo.h
    VersionInfo.rc
    # data
    data/HalfSpace.cpp
    data/HalfSpace.h
    data/HashableEdge.cpp
    data/HashableEdge.h
    data/Mesh.cpp
    data/Mesh.h
    data/Scene.cpp
    data/Scene.h
    data/Shape2D.cpp
    data/Shape2D.h
    data/Triangle.cpp
    data/Triangle.h
    # utilities
    utilities/LoopsFromEdges.h
    utilities/StringUtilities.cpp
    utilities/StringUtilities.h
    utilities/Constrained2DTriangulation.cpp
    utilities/Constrained2DTriangulation.h
    # lua
    lua/CommandCreator.cpp
    lua/CommandCreator.h
    lua/LogFunctions.cpp
    lua/LogFunctions.h
    lua/LuaResources.h
    lua/LuaResources.rc
    lua/LuaUtilities.cpp
    lua/LuaUtilities.h
    lua/Runner.cpp
    lua/Runner.h
    lua/VersionCheck.cpp
    lua/VersionCheck.h
    lua/types/AbstractListType.cpp
    lua/types/AbstractListType.h
    lua/types/AbstractType.cpp
    lua/types/AbstractType.h
    lua/types/CommandType.cpp
    lua/types/CommandType.h
    lua/types/FloatListType.cpp
    lua/types/FloatListType.h
    lua/types/GlmMat4Type.cpp
    lua/types/GlmMat4Type.h
    lua/types/GlmUVec3Type.cpp
    lua/types/GlmUVec3Type.h
    lua/types/GlmVec2Type.cpp
    lua/types/GlmVec2Type.h
    lua/types/GlmVec3ListListType.cpp
    lua/types/GlmVec3ListListType.h
    lua/types/GlmVec3ListType.cpp
    lua/types/GlmVec3ListType.h
    lua/types/GlmVec3Type.cpp
    lua/types/GlmVec3Type.h
    lua/types/HalfSpaceType.cpp
    lua/types/HalfSpaceType.h
    lua/types/IndexListListType.cpp
    lua/types/IndexListListType.h
    lua/types/IndexListType.cpp
    lua/types/IndexListType.h
    lua/types/MeshListType.cpp
    lua/types/MeshListType.h
    lua/types/MeshType.cpp
    lua/types/MeshType.h
    lua/types/SceneType.cpp
    lua/types/SceneType.h
    # commands
    commands/AbstractCommand.cpp
    commands/AbstractCommand.h
    commands/CommandFactory.cpp
    commands/CommandFactory.h
    commands/CommandRegistration.cpp
    commands/CommandRegistration.h
    commands/CommandRegistration.inc
    commands/Parameter.cpp
    commands/Parameter.h
    commands/ParameterBinding.cpp
    commands/ParameterBinding.h
    commands/compute/LinearColorMap.cpp
    commands/compute/LinearColorMap.h
    commands/compute/OpenBorder.cpp
    commands/compute/OpenBorder.h
    commands/compute/SplitByEdges.cpp
    commands/compute/SplitByEdges.h
    commands/compute/VertexEdgeDistance.cpp
    commands/compute/VertexEdgeDistance.h
    commands/compute/VertexEdgeDistanceToCut.cpp
    commands/compute/VertexEdgeDistanceToCut.h
    commands/compute/VertexNormals.cpp
    commands/compute/VertexNormals.h
    commands/edit/CloseLoopWithPin.cpp
    commands/edit/CloseLoopWithPin.h
    commands/edit/CutHalfSpace.cpp
    commands/edit/CutHalfSpace.h
    commands/edit/CutPlaneLoop.cpp
    commands/edit/CutPlaneLoop.h
    commands/edit/DisplacementNoise.cpp
    commands/edit/DisplacementNoise.h
    commands/edit/InvertVertexSelection.cpp
    commands/edit/InvertVertexSelection.h
    commands/edit/SelectConnectedComponentVertices.cpp
    commands/edit/SelectConnectedComponentVertices.h
    commands/edit/Subdivision.cpp
    commands/edit/Subdivision.h
    commands/generator/Cuboid.cpp
    commands/generator/Cuboid.h
    commands/generator/Icosahedron.cpp
    commands/generator/Icosahedron.h
    commands/generator/Octahedron.cpp
    commands/generator/Octahedron.h
    commands/generator/SphereIco.cpp
    commands/generator/SphereIco.h
    commands/io/Model3mfReader.cpp
    commands/io/Model3mfReader.h
    commands/io/ObjReader.cpp
    commands/io/ObjReader.h
    commands/io/ObjWriter.cpp
    commands/io/ObjWriter.h
    commands/io/PlyReader.cpp
    commands/io/PlyReader.h
    commands/io/PlyWriter.cpp
    commands/io/PlyWriter.h
    commands/io/StlReader.cpp
    commands/io/StlReader.h
    commands/io/StlWriter.cpp
    commands/io/StlWriter.h
    # 3rd Party
    3rd-party/xyz_math/xyz_math.lua
    3rd-party/xyz_math/README.md
)

set_source_files_properties(
    # 3rd Party
    3rd-party/xyz_math/xyz_math.lua
    3rd-party/xyz_math/README.md
    PROPERTIES HEADER_FILE_ONLY TRUE
)
source_group("/3rd-party/xyz_math" FILES
    # 3rd Party
    3rd-party/xyz_math/xyz_math.lua
    3rd-party/xyz_math/README.md
)
source_group("/commands" FILES
    # 3rd Party
    commands/CommandRegistration.inc
)

find_package(SGrottelSimpleLog REQUIRED)

find_package(SGrottelYaclap REQUIRED)

find_package(GLM REQUIRED)

set(LuaToolset "v143")
find_package(Lua REQUIRED)

find_package(CGAL CONFIG REQUIRED)

find_package(lib3mf CONFIG REQUIRED)
# lib3mf injects a /W3, which clashes with my project setting /W4
# so, I remove it
get_target_property(L3MF_OPTS_BEFORE lib3mf::lib3mf INTERFACE_COMPILE_OPTIONS)
# message(STATUS "lib3mf::lib3mf INTERFACE_COMPILE_OPTIONS (before) = '${L3MF_OPTS_BEFORE}'")
if (L3MF_OPTS_BEFORE)
    set(L3MF_OPTS_CLEAN "${L3MF_OPTS_BEFORE}")
    list(FILTER L3MF_OPTS_CLEAN EXCLUDE REGEX "/W[0-4]") # Removes all /W[0-4] flags (including /W3)
    set_target_properties(lib3mf::lib3mf PROPERTIES INTERFACE_COMPILE_OPTIONS "${L3MF_OPTS_CLEAN}")
endif()
# get_target_property(L3MF_OPTS_AFTER lib3mf::lib3mf INTERFACE_COMPILE_OPTIONS)
# message(STATUS "lib3mf::lib3mf INTERFACE_COMPILE_OPTIONS (after) = '${L3MF_OPTS_AFTER}'")

# Modern compile features
target_compile_features(MeshProc PRIVATE cxx_std_20)
target_compile_options(MeshProc PRIVATE /std:c++20 /Zc:__cplusplus)

# Recommended warnings (portable)
if (MSVC)
    target_compile_options(MeshProc PRIVATE /W4 /permissive-)
else()
    target_compile_options(MeshProc PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(MeshProc PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN VC_EXTRALEAN)

target_include_directories(MeshProc
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)
target_link_libraries(MeshProc
    PRIVATE
        SGrottelSimpleLog
        SGrottelYaclap
        GLM
        Lua::Lua
        CGAL::CGAL
        lib3mf::lib3mf
)

# Required for TARGET_RUNTIME_DLLS on MSVC
set_property(TARGET MeshProc PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:MeshProc>)

set(VCPKG_BIN_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin")
file(GLOB VCPKG_DLLS "${VCPKG_BIN_DIR}/*.dll")
if (VCPKG_DLLS)
    add_custom_command(TARGET MeshProc POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VCPKG_DLLS} $<TARGET_FILE_DIR:MeshProc>
    )
endif()
