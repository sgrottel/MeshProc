#include "CmdLineArgs.h"
#include "CubeGenerator.h"
#include "FlatSkirt.h"
#include "Mesh.h"
#include "OpenBorder.h"
#include "StlReader.h"
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

	std::shared_ptr<Mesh> mesh;
	if (std::filesystem::is_regular_file(cmdLine.input))
	{
		StlReader reader{ log };
		mesh = reader.Load(cmdLine.input);
	}

	if (mesh->triangles.empty())
	{
		log.Warning("Mesh is empty");
		return 1;
	}

	{
		OpenBorder openBorder{ log };
		auto border = openBorder.Find(mesh);

		if (!border.empty())
		{
			log.Message("Found %d open borders", static_cast<int>(border.size()));

			for (auto const& loop : border)
			{
				if (loop.size() < 3)
				{
					log.Warning("Skipping degenerated loop");
					continue;
				}

				FlatSkirt skirt{ log };
				std::vector<uint32_t> newLoop = skirt.AddSkirt(mesh, loop);

				// TODO: detect loop self-intersection in projected 2d space
				// if intersecting -> resolve by collapsing both offending edges,
				//                    remove degenerated triangles,
				//                    flip triangles if necessary (should be on one side).
				//                    repeat until no longer self-intersecting.
				//                    then break loops.

				// TODO: fill hole in projected 2d space.

			}
		}
	}

	StlWriter writer{ log };
	writer.Save(L"cube.stl", mesh);

	// TODO: Implement

	return 0;
}
