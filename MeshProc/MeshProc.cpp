#include "CmdLineArgs.h"
#include "CubeGenerator.h"
#include "FlatSkirt.h"
#include "Mesh.h"
#include "OpenBorder.h"
#include "Scene.h"
#include "StlReader.h"
#include "StlWriter.h"

#include <SimpleLog/SimpleLog.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

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
	std::shared_ptr<Scene> scene;

	if (!cmdLine.inputs.empty())
	{
		if (std::filesystem::is_regular_file(cmdLine.inputs.front()))
		{
			StlReader reader{ log };
			mesh = reader.Load(cmdLine.inputs.front());

			if (mesh->triangles.empty())
			{
				log.Warning("Mesh is empty");
				return 1;
			}
		}

		if (cmdLine.inputs.size() > 1)
		{
			scene = std::make_shared<Scene>();
			scene->m_meshes.push_back({ mesh, glm::mat4(1.0) });

			StlReader reader{ log };
			for (auto it = cmdLine.inputs.begin() + 1; it != cmdLine.inputs.end(); ++it)
			{
				std::shared_ptr<Mesh> m = reader.Load(*it);
				if (m->triangles.empty())
				{
					log.Warning("Mesh is empty");
					continue;
				}

				glm::mat4 ma(1.0f);

				switch (scene->m_meshes.size())
				{
				case 1:
				{
					glm::mat4 scale = glm::scale(glm::vec3(0.485f));
					glm::mat4 rotate = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					glm::mat4 translate = glm::translate(glm::vec3(
						(-57.466003f + -1.082000f) / -2.0f,
						-43.580002f,
						(-37.636002f + 8.714001f) / -2.0f));
					// scale([0.485,0.485,0.485])
					// rotate([-90,0,0])
					// translate([
					//     (-57.466003 + -1.082000) / -2.0,
					//     -43.580002,
					//     (-37.636002 + 8.714001) / -2.0
					//     ])
					ma = scale * rotate * translate;
				}
					break;
				case 2:
				{
					glm::mat4 translate2 = glm::translate(glm::vec3(-6.0f, 6.0f, 20.0f));
					glm::mat4 rotate2 = glm::rotate(glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
					glm::mat4 scale = glm::scale(glm::vec3(0.485f));
					glm::mat4 rotate = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					glm::mat4 translate = glm::translate(glm::vec3(
						(-34.432003f + 39.858002f) / -2.0f,
						0,
						(-28.958002f + 44.544003f) / -2.0f));
					// translate([-6,6,20])
					// rotate([0,0,15])
					// scale([0.485,0.485,0.485])
					// rotate([-90,0,0])
					// translate([
					//     (-34.432003 + 39.858002) / -2.0,
					//     0,
					//     (-28.958002 + 44.544003) / -2.0
					//     ])
					ma = translate2 * rotate2 * scale * rotate * translate;
				}
					break;
				}

				scene->m_meshes.push_back({ m, ma });
			}
		}

	}
	else
	{
		mesh = CubeGenerator::Create(3, 4, 5);
	}

	/*

	std::vector<glm::vec3> faceNormals;
	faceNormals.resize(mesh->triangles.size());
	std::transform(mesh->triangles.begin(), mesh->triangles.end(), faceNormals.begin(), [&mesh](auto const& tri) { return tri.CalcNormal(mesh->vertices); });

	Chamfer chamfer{ log };
	chamfer.ChamferEdge(mesh, 0, 1, 0.2f, faceNormals);
	*/

	/*
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

				// fill hole via pin
				uint32_t pinIdx = static_cast<uint32_t>(mesh->vertices.size());
				mesh->vertices.push_back(skirt.GetCenter() + skirt.GetZDir() * skirt.GetZDist());
				mesh->triangles.reserve(mesh->triangles.size() + newLoop.size());
				for (size_t i = 0; i < newLoop.size(); ++i)
				{
					mesh->triangles.push_back({ newLoop[i], newLoop[(i + 1) % newLoop.size()], pinIdx });
				}

			}
		}
	}
	*/

	if (!scene)
	{
		scene = std::make_shared<Scene>();
	}
	if (scene->m_meshes.empty())
	{
		scene->m_meshes.push_back({ mesh, glm::mat4(1.0) });
	}

	StlWriter writer{ log };
	writer.Save(L"cube.stl", scene);

	// TODO: Implement

	return 0;
}
