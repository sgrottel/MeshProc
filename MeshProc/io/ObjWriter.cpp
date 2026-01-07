#include "ObjWriter.h"

#include <SimpleLog/SimpleLog.hpp>

#include <cstdio>

using namespace meshproc;
using namespace meshproc::io;

ObjWriter::ObjWriter(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::In, ParamType::Scene>("Scene", m_scene);
}

bool ObjWriter::Invoke()
{
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
	fprintf(file, "# MeshProc ObjWriter\n");

	// export whole scene as single mesh!

	// all verticies
	for (auto const& mesh : m_scene->m_meshes)
	{
		for (auto const& vertex : mesh.first->vertices)
		{
			glm::vec4 v = mesh.second * glm::vec4{ vertex, 1.0f };
			v *= 1.0f / v.w;
			fprintf(file, "v %f %f %f\n", v.x, v.y, v.z);
		}
	}

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
