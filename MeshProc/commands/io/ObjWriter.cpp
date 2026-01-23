#include "ObjWriter.h"

#include <SimpleLog/SimpleLog.hpp>

#include <cstdio>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::io;

ObjWriter::ObjWriter(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::In, ParamType::Scene>("Scene", m_scene);
	AddParamBinding<ParamMode::In, ParamType::Vec3ListList>("VertexColors", m_vertexColors);
}

bool ObjWriter::Invoke()
{
	if (!m_scene)
	{
		Log().Error(L"'Scene' not set");
		return false;
	}

	if (m_vertexColors)
	{
		if (m_scene->m_meshes.size() != m_vertexColors->size())
		{
			Log().Error(L"Inconsistent vertex colors; scene with %d meshes; vertex colors for %d meshes", m_scene->m_meshes.size(), m_vertexColors->size());
		}
		for (size_t i = 0; i < m_scene->m_meshes.size(); ++i)
		{
			auto const& g = m_scene->m_meshes.at(i).first->vertices;
			auto const& c = m_vertexColors->at(i);
			if (g.size() != c->size() && c->size() != 0)
			{
				Log().Error(L"Inconsistent vertex colors; Scene mesh %d has %d vertices, but %d vertex color entries", i, g.size(), c->size());
			}
		}
	}

	FILE* file = nullptr;
	errno_t r = _wfopen_s(&file, m_path.c_str(), L"wb");
	if (r != 0) {
		wchar_t errMsg[95]{};
		_wcserror_s(errMsg, r);
		Log().Error(L"Failed to open \"%s\": %s (%d)", m_path.c_str(), errMsg, static_cast<int>(r));
		return false;
	}
	if (file == nullptr) {
		Log().Error(L"Failed to open \"%s\": returned nullptr", m_path.c_str());
		return false;
	}

	Log().Message(L"Writing OBJ: %s", m_path.c_str());
	fprintf(file, "# MeshProc ObjWriter");

	// export whole scene as single mesh!

	// all verticies
	for (size_t i = 0; i < m_scene->m_meshes.size(); ++i)
	{
		auto const& mesh = m_scene->m_meshes.at(i);
		std::shared_ptr<std::vector<glm::vec3>> col = m_vertexColors ? m_vertexColors->at(i) : nullptr;
		if (col && col->size() != mesh.first->vertices.size())
		{
			col.reset();
		}

		for (size_t j = 0; j < mesh.first->vertices.size(); ++j)
		{
			glm::vec4 v = mesh.second * glm::vec4{ mesh.first->vertices.at(j), 1.0f };
			v *= 1.0f / v.w;
			fprintf(file, "\nv %f %f %f", v.x, v.y, v.z);
			if (col)
			{
				auto c = col->at(j);
				fprintf(file, " %f %f %f",
					std::clamp(c.x, 0.0f, 1.0f),
					std::clamp(c.y, 0.0f, 1.0f),
					std::clamp(c.z, 0.0f, 1.0f));
			}
			else if (m_vertexColors)
			{
				fprintf(file, " 0 0 0");
			}
		}
	}

	fprintf(file, "\n");

	// all trianges
	uint32_t vertexOffset = 0;
	for (auto const& mesh : m_scene->m_meshes)
	{
		for (data::Triangle const& t : mesh.first->triangles)
		{
			fprintf(file, "f %d %d %d\n", t[0] + vertexOffset + 1, t[1] + vertexOffset + 1, t[2] + vertexOffset + 1);
		}
		vertexOffset += static_cast<int>(mesh.first->vertices.size());
	}

	// done.
	fclose(file);
	return true;
}
