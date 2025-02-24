#include "CmdLineArgs.h"
#include "CommandFactory.h"
#include "CommandRegistration.h"

#include "data/Mesh.h"
#include "data/Scene.h"
#include "FlatSkirt.h"
#include "generator/Cube.h"
#include "generator/Icosahedron.h"
#include "generator/SphereIco.h"
#include "io/ObjReader.h"
#include "io/StlReader.h"
#include "io/StlWriter.h"
#include "OpenBorder.h"

#include <SimpleLog/SimpleLog.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <memory>
#include <cwctype>

using namespace meshproc;
using namespace meshproc::data;

namespace
{
	template<typename T>
	bool Invoke(T& obj)
	{
		bool r;
		obj.PreInvoke();
		try
		{
			r = obj.Invoke();
		}
		catch (...)
		{
			obj.PostInvoke();
			throw;
		}
		obj.PostInvoke();
		return r;
	}

}

int wmain(int argc, wchar_t **argv)
{
	sgrottel::NullLog nullLog;
	sgrottel::EchoingSimpleLog log{ nullLog };

	CmdLineArgs cmdLine;
	if (!cmdLine.Parse(log, argc, argv)) {
		return 1;
	}

	meshproc::CommandFactory cmdFactory{ log };
	meshproc::CommandRegistration(cmdFactory, log);

	std::shared_ptr<Scene> scene = std::make_shared<Scene>();

	if (!cmdLine.inputs.empty())
	{
		for (auto const& inputPath : cmdLine.inputs)
		{
			if (!std::filesystem::exists(inputPath))
			{
				log.Error(L"Specified input file does not seem to exist: %s", inputPath.wstring().c_str());
				continue;
			}

			std::wstring ext = inputPath.extension().wstring();
			std::transform(ext.begin(), ext.end(), ext.begin(), &std::towlower);

			glm::mat4 placement{ 1.0f };

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
				placement = scale * rotate * translate;
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
				placement = translate2 * rotate2 * scale * rotate * translate;
			}
				break;
			}

			if (ext == L".stl")
			{
				io::StlReader reader{ log };
				reader.Path.Put() = cmdLine.inputs.front();
				if (!Invoke(reader))
				{
					log.Error("StlReader.Invoke failed");
					continue;
				}

				scene->m_meshes.push_back({ reader.Mesh.Get(), placement });

			}
			else if (ext == L".obj")
			{
				io::ObjReader reader{ log };
				reader.Path.Put() = cmdLine.inputs.front();
				if (!Invoke(reader))
				{
					log.Error("ObjReader.Invoke failed");
					continue;
				}

				scene->m_meshes.push_back({ reader.Mesh.Get(), placement });

			}
			else
			{
				log.Error(L"Unknown input file format: %s", ext.c_str());
			}
		}

	}
	else
	{
		/*
		generator::Cube cube{ log };
		cube.SizeX.Put() = 3;
		cube.SizeY.Put() = 4;
		cube.SizeZ.Put() = 5;
		if (!Invoke(cube))
		{
			log.Error("CubeGenerator.Invoke failed");
			return 1;
		}
		scene->m_meshes.push_back({ cube.Mesh.Get(), glm::mat4{1.0f} });
		*/
		/*
		generator::Icosahedron ico{ log };
		*/
		generator::SphereIco ico{ log };
		ico.Iterations.Put() = 4;

		if (!Invoke(ico))
		{
			log.Error("CubeGenerator.Invoke failed");
			return 1;
		}
		scene->m_meshes.push_back({ ico.Mesh.Get(), glm::mat4{1.0f} });
	}

	/*

	std::vector<glm::vec3> faceNormals;
	faceNormals.resize(mesh->triangles.size());
	std::transform(mesh->triangles.begin(), mesh->triangles.end(), faceNormals.begin(), [&mesh](auto const& tri) { return tri.CalcNormal(mesh->vertices); });

	Chamfer chamfer{ log };
	chamfer.ChamferEdge(mesh, 0, 1, 0.2f, faceNormals);
	*/

	/*
	if (scene->m_meshes.size() == 1)
	{
		OpenBorder openBorder{ log };
		openBorder.Mesh.Put() = scene->m_meshes.front().first;
		if (!Invoke(openBorder))
		{
			log.Error("OpenBorder.Invoke failed");
			return 1;
		}

		if (!openBorder.EdgeLists.Get().empty())
		{
			std::vector<std::vector<uint32_t>> loops;
			std::swap(loops, openBorder.EdgeLists.Put());

			log.Message("Found %d open borders", static_cast<int>(loops.size()));

			std::sort(loops.begin(), loops.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });

			glm::vec3 closingDir;

			{
				FlatSkirt skirt{ log };
				skirt.Mesh.Put() = scene->m_meshes.front().first;
				std::swap(skirt.Loop.Put(), loops.front());
				if (!Invoke(skirt))
				{
					log.Error("skirt.Invoke failed");
					return 1;
				}
				std::swap(loops.front(), skirt.NewLoop.Put());

				closingDir = skirt.ZDir.Get() * skirt.ZDist.Get();
			}

			for (auto& loop : loops)
			{
				if (loop.size() < 3)
				{
					log.Warning("Skipping degenerated loop");
					continue;
				}

				// fill hole via pin
				auto& mesh = scene->m_meshes.front().first;
				uint32_t pinIdx = static_cast<uint32_t>(mesh->vertices.size());

				glm::vec3 center{ 0.0 };
				for (uint32_t i : loop)
				{
					center += mesh->vertices[i];
				}
				center /= loop.size();

				mesh->vertices.push_back(center + closingDir);

				mesh->triangles.reserve(mesh->triangles.size() + loop.size());
				for (size_t i = 0; i < loop.size(); ++i)
				{
					mesh->triangles.push_back({ loop[i], loop[(i + 1) % loop.size()], pinIdx });
				}

			}

		}
	}
	*/

	if (scene->m_meshes.empty())
	{
		log.Warning("Scene is empty");
	}

	io::StlWriter writer{ log };
	writer.Path.Put() = L"cube.stl";
	writer.Scene.Put() = scene;
	if (!Invoke(writer))
	{
		log.Error("StlWriter.Invoke failed");
		return 1;
	}

	// TODO: Implement

	return 0;
}
