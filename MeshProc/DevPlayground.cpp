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

#include <glm/glm.hpp>

using namespace meshproc;

void DevPlayground(const sgrottel::ISimpleLog& log)
{
	std::shared_ptr<data::Mesh> mesh;
	glm::mat4 objMat{ 1.0f };

	{
		generator::CrystalGrain grain{ log };
		grain.Invoke();
		mesh = grain.GetMesh();
	}
	
	/*	{
		io::ObjReader reader{ log };
		reader.SetPath(L"in.obj");
		reader.Invoke();
		mesh = reader.GetMesh();
	}
	ParamTypeInfo_t<ParamType::MultiVertexSelection> openLoops;
	{
		OpenBorder border{ log };
		border.SetMesh(mesh);
		border.Invoke();
		openLoops = border.GetEdgeLists();
	}
	if (openLoops->size() == 2)
	{
		std::sort(openLoops->begin(), openLoops->end(), [](auto const& a, auto const& b) { return a->size() > b->size(); });
		FlatSkirt skirt{ log };
		skirt.SetMesh(mesh);

		skirt.SetLoop(openLoops->at(0));
		skirt.Invoke();
		openLoops->at(0) = skirt.GetNewLoop();

		glm::vec3 backC = skirt.GetCenter();
		glm::vec3 backDir = skirt.GetZDir();

		objMat = glm::mat4{
			glm::vec4{ skirt.GetX2D(), 0.0f },
			glm::vec4{ skirt.GetY2D(), 0.0f },
			glm::vec4{ glm::cross(skirt.GetY2D(), skirt.GetX2D()), 0.0f },
			glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }
		};

		skirt.SetLoop(openLoops->at(1));
		skirt.Invoke();
		openLoops->at(1) = skirt.GetNewLoop();

		glm::vec3 c = skirt.GetCenter();
		glm::vec3 x = skirt.GetX2D();
		glm::vec3 y = skirt.GetY2D();
		glm::vec3 dir = skirt.GetZDir();
		float dist = skirt.GetZDist();

		std::vector<float> ang;
		ang.reserve(openLoops->at(1)->size());
		float d = 0.0f;
		for (uint32_t idx : *openLoops->at(1))
		{
			glm::vec3 v = mesh->vertices[idx] - c;
			float l = glm::length(v);

			v /= l;
			float vx = glm::dot(v, x);
			float vy = glm::dot(v, y);

			ang.push_back(std::atan2f(vx, vy));

			if (d < l)
			{
				d = l;
			}
		}
		assert(ang.size() == openLoops->at(1)->size());

		bool asc = true;
		bool repair = false;
		{
			size_t cntGrow = 0;
			size_t cntShrink = 0;
			for (size_t i = 0; i < ang.size(); ++i)
			{
				if (ang[(i + 1) % ang.size()] > ang[i])
				{
					cntGrow++;
				}
				else
				{
					cntShrink++;
				}
			}
			if (cntShrink > cntGrow)
			{
				asc = false;
			}
			if (cntShrink > 1 && cntGrow > 1)
			{
				repair = true;
			}
		}

		while (repair)
		{
			repair = false;
			for (size_t i1 = 0; i1 < ang.size(); ++i1)
			{
				const size_t i2 = (i1 + 1) % ang.size();
				if ((!asc && (ang[i2] > ang[i1]))
					|| (asc && (ang[i2] < ang[i1])))
				{
					if (std::abs(ang[i1] - ang[i2]) > 3.14159)
					{
						// likely the wraparound, so lets ignore that.
						continue;
					}
					// Would need to check if swapping i1<->i0 or i2<->i3 would resolve the issue as well, with a smaller error
					std::swap(ang[i1], ang[i2]);
					repair = true;
				}
			}
		}

		d *= 1.1f;
		for (size_t i = 0; i < ang.size(); ++i)
		{
			uint32_t idx = openLoops->at(1)->at(i);
			mesh->vertices[idx] = c + (y * std::cos(ang[i]) + x * std::sin(ang[i])) * d;
		}

		skirt.SetLoop(openLoops->at(1));
		skirt.Invoke();
		openLoops->at(1) = skirt.GetNewLoop();

		for (uint32_t idx : *openLoops->at(1))
		{
			float l = 0.6f * glm::dot(backC - mesh->vertices[idx], dir);
			mesh->vertices[idx] += dir * l;
		}

		CloseLoopWithPin close{ log };
		close.SetMesh(mesh);
		close.SetLoop(openLoops->at(1));
		close.Invoke();

		close.SetLoop(openLoops->at(0));
		close.Invoke();

		mesh->vertices[close.GetNewVertexIndex()] += backDir;
	}
	*/

	{
		std::shared_ptr<data::Scene> scene = std::make_shared<data::Scene>();
		scene->m_meshes.push_back({ mesh, objMat });

		io::StlWriter writer{ log };
		writer.SetPath(L"out.stl");
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
