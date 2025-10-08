#include "ObjMeshWriter.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/gtc/type_ptr.hpp>

//#include <cstdio>

using namespace meshproc;
using namespace meshproc::io;

ObjMeshWriter::ObjMeshWriter(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::ListOfVec3>("VertexColors", m_vertColors);
}

bool ObjMeshWriter::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (m_vertColors && (m_mesh->vertices.size() != m_vertColors->size()))
	{
		Log().Error("VertexColors is set, but size not equal to number of vertices in Mesh");
		return false;
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

	Log().Message(L"Writing OBJ mesh: %s", m_path.c_str());
	fprintf(file, "# MeshProc ObjMeshWriter");

	auto vi = m_mesh->vertices.begin();
	auto ve = m_mesh->vertices.end();
	auto vci = m_vertColors ? m_vertColors->begin() : ve;
	auto vce = m_vertColors ? m_vertColors->end() : ve;

	for (; vi != ve; ++vi)
	{
		fprintf(file, "\nv %f %f %f", vi->x, vi->y, vi->z);
		if (vci != vce)
		{
			fprintf(file, " %f %f %f",
				std::clamp(vci->r, 0.0f, 1.0f),
				std::clamp(vci->g, 0.0f, 1.0f),
				std::clamp(vci->b, 0.0f, 1.0f));
		}
		if (vci != vce) ++vci;
	}

	fprintf(file, "\n");
	for (auto const& t : m_mesh->triangles)
	{
		fprintf(file, "f %d %d %d\n", t[0] + 1, t[1] + 1, t[2] + 1);
	}

	// done.
	fclose(file);
	return true;
}
