
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
    # lua
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
    # commands/edit/CutHalfSpace.cpp
    # commands/edit/CutHalfSpace.h
    commands/edit/DisplacementNoise.cpp
    commands/edit/DisplacementNoise.h
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
)

find_package(SGrottelSimpleLog REQUIRED)
find_package(SGrottelYaclap REQUIRED)
find_package(GLM REQUIRED)

# Modern compile features
target_compile_features(MeshProc PRIVATE cxx_std_20)

# Recommended warnings (portable)
if (MSVC)
    target_compile_options(MeshProc PRIVATE /W4 /permissive-)
else()
    target_compile_options(MeshProc PRIVATE -Wall -Wextra -Wpedantic)
endif()

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
)

# If you later add include dirs or libraries:
# target_include_directories(myapp PRIVATE include)
# target_link_libraries(myapp PRIVATE mylib)
