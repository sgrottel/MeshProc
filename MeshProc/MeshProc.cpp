#include "CmdLineArgs.h"
#include "CubeGenerator.h"
#include "Mesh.h"
#include "StlWriter.h"

#include <SimpleLog/SimpleLog.hpp>

#include <memory>

int wmain(int argc, wchar_t **argv)
{
	sgrottel::NullLog nullLog;
	sgrottel::EchoingSimpleLog log{ nullLog };

	CmdLineArgs cmdLine;
	if (!cmdLine.Parse(log, argc, argv)) {
		return 1;
	}

	std::shared_ptr<Mesh> cube = CubeGenerator::Create(3, 4, 5);
	StlWriter writer{ log };

	std::vector<glm::vec3> faceNormals;
	faceNormals.resize(cube->triangles.size());
	std::transform(cube->triangles.begin(), cube->triangles.end(), faceNormals.begin(), [&cube](auto const& tri) { return tri.CalcNormal(cube->vertices); });

	writer.Save(L"cube.stl", cube);

	// TODO: Implement

	return 0;
}
