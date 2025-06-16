#include "DevPlayground.h"

#include "data/Mesh.h"
#include "data/Scene.h"
#include "io/ObjReader.h"
#include "OpenBorder.h"
#include "FlatSkirt.h"
#include "CloseLoopWithPin.h"
#include "io/StlWriter.h"
#include "generator/CrystalGrain.h"

#include <cassert>
#include <functional>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace meshproc;

void DevPlayground(const sgrottel::ISimpleLog& log)
{
	std::shared_ptr<data::Mesh> mesh = std::make_shared<data::Mesh>();
	glm::mat4 objMat{ 1.0f };
	objMat = glm::translate(objMat, glm::vec3{ 0, 0, -22.5 });

	std::vector<glm::vec2> flatCoords;
	
	std::vector<glm::vec2> flatDirs;

	constexpr float M_PI = 3.1415926535f;

	auto insertEdge = [&](const glm::vec2& next)
		{
			glm::vec2 prev = flatCoords.back();
			glm::vec2 prevDir = flatDirs.back();
			const int len = static_cast<int>(glm::distance(prev, next));
			for (int i = 1; i < len; ++i)
			{
				const float b = static_cast<float>(i) / static_cast<float>(len);
				const float a = 1.0f - b;

				flatDirs.push_back(prevDir);
				flatCoords.push_back(prev * a + next * b);
			}
		};

	auto pushCorner = [&](float x, float y, float r, float ang)
		{
			bool doEdge = flatCoords.size() > 0;

			constexpr int steps = 5;
			for (int i = steps; i >= 0; --i)
			{
				const float a = ang + i * 90.0f / steps;
				const float ar = a * M_PI / 180.0f;
				const float s = std::sin(ar);
				const float c = std::cos(ar);
				
				const glm::vec2 coord{ x - c * r, y - s * r };
				if (doEdge)
				{
					insertEdge(coord);
				}
				flatDirs.push_back(glm::vec2{ -c, -s });
				flatCoords.push_back(coord);
			}
		};

	pushCorner(1, 1, 2, 0);
	const size_t cornerLen = flatCoords.size();
	pushCorner(1, 91, 2, 270);
	pushCorner(46, 91, 2, 180);
	pushCorner(46, 1, 2, 90);
	insertEdge(flatCoords.front());

	assert(flatCoords.size() == flatDirs.size());

	std::random_device r;
	std::default_random_engine randEng(r());
	std::uniform_real_distribution<float> randDist(0.0f, 1.0f);

	auto zero = []() { return 0.0f; };
	auto minus15 = []() { return -1.5f; };
	auto rand05 = [&]() { return randDist(randEng) * 0.5f; };

	auto pushCoords = [&](float z, std::function<float()> offset)
		{
			for (size_t i = 0; i < flatCoords.size(); ++i)
			{
				const auto& c = flatCoords[i];
				const auto& d = flatDirs[i];
				const float r = offset();

				mesh->vertices.push_back(glm::vec3{ c + d * r, z });
			}
		};
	auto pushWall = [&]()
		{
			uint32_t size = static_cast<uint32_t>(flatCoords.size());
			assert(size > 2);
			uint32_t start = static_cast<uint32_t>(mesh->vertices.size() - 2 * size);
			for (uint32_t i = 0; i < size; ++i)
			{
				const uint32_t v11 = start + i;
				const uint32_t v12 = start + (i + 1) % size;
				const uint32_t v21 = start + size + i;
				const uint32_t v22 = start + size + (i + 1) % size;

				if (((v11 / size) + (v11 % size)) % 2)
				{
					mesh->triangles.push_back(data::Triangle{ v12, v11, v21 });
					mesh->triangles.push_back(data::Triangle{ v12, v21, v22 });
				}
				else
				{
					mesh->triangles.push_back(data::Triangle{ v12, v11, v22 });
					mesh->triangles.push_back(data::Triangle{ v11, v21, v22 });
				}
			}
		};
	auto pushCap = [&](bool flip)
		{
			uint32_t size = static_cast<uint32_t>(flatCoords.size());
			assert(size > 2);
			uint32_t start = static_cast<uint32_t>(mesh->vertices.size() - size);
			for (uint32_t i = 2; i < size; ++i)
			{
				const uint32_t v1 = start;
				const uint32_t v2 = start + i - 1;
				const uint32_t v3 = start + i;

				mesh->triangles.push_back(data::Triangle{ v1, flip ? v3 : v2, flip ? v2 : v3 });
			}
		};

	pushCoords(0.0f, zero);
//	pushCap(false);
	for (int z = 1; z < 18; ++z)
	{
		pushCoords(static_cast<float>(z), rand05);
		pushWall();
	}
	pushCoords(18.0f, zero);
	pushWall();
	pushCoords(18.0f, minus15);
	pushWall();
	pushCoords(0.0, minus15);
	pushWall();
	pushCoords(0.0f, zero);
	pushWall();
	//	pushCap(true);

	{
		std::shared_ptr<data::Scene> scene = std::make_shared<data::Scene>();
		scene->m_meshes.push_back({ mesh, objMat });

		io::StlWriter writer{ log };
		writer.SetPath(L"randHull.stl");
		writer.SetScene(scene);
		writer.Invoke();
	}

#if 0

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

#endif
}
