#include "PlyWriter.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/gtc/type_ptr.hpp>

//#include <cstdio>

using namespace meshproc;
using namespace meshproc::io;

PlyWriter::PlyWriter(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::In, ParamType::Scene>("Scene", m_scene);
}

bool PlyWriter::Invoke()
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

	Log().Message(L"Writing PLY: %s", m_path.c_str());

	// following the description of
	// http://gamma.cs.unc.edu/POWERPLANT/papers/ply.pdf

	// count elements uint32
	int triCnt = 0, vertCnt = 0;
	for (auto const& mesh : m_scene->m_meshes)
	{
		vertCnt += static_cast<int>(mesh.first->vertices.size());
		triCnt += static_cast<int>(mesh.first->triangles.size());
	}

	// header
	fprintf(file, "ply\n");
	fprintf(file, "format binary_little_endian 1.0\n");

	fprintf(file, "element vertex %d\n", vertCnt);
	fprintf(file, "property float x\n");
	fprintf(file, "property float y\n");
	fprintf(file, "property float z\n");

	fprintf(file, "element face %d\n", triCnt);
	fprintf(file, "property list uchar uint vertex_indices\n");

	fprintf(file, "end_header\n");

	// all verticies
	for (auto const& mesh : m_scene->m_meshes)
	{
		for (auto const& vertex : mesh.first->vertices)
		{
			glm::vec4 v = mesh.second * glm::vec4{ vertex, 1.0f };
			v *= 1.0f / v.w;
			fwrite(glm::value_ptr(v), 4, 3, file);
		}
	}

	// all trianges
	uint32_t vertexOffset = 0;
	uint8_t triBuf[1 + 3 * 4] = { 3,0,0,0,0,0,0,0,0,0,0,0,0 };
	uint32_t* triCoordBuf = reinterpret_cast<uint32_t*>(triBuf + 1);
	for (auto const& mesh : m_scene->m_meshes)
	{
		for (data::Triangle const& t : mesh.first->triangles)
		{
			for (int i = 0; i < 3; ++i)
			{
				triCoordBuf[i] = vertexOffset + t[i];
			}
			fwrite(triBuf, 1, 1 + 3 * 4, file);
		}
		vertexOffset += static_cast<int>(mesh.first->vertices.size());
	}

	// done.
	fclose(file);
	Log().Detail(L"Written %d triangles and %d vertices to %s", triCnt, vertCnt, m_path.c_str());
	return true;
}
