#include "StlWriter.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <cstdio>

using namespace meshproc;
using namespace meshproc::io;

StlWriter::StlWriter(sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
}

bool StlWriter::Invoke()
{
	std::wstring wfilename{ Path.Get().wstring() };
	FILE* file = nullptr;
	errno_t r = _wfopen_s(&file, wfilename.c_str(), L"wb");
	if (r != 0) {
		Log().Error(L"Failed to open \"%s\": %d", wfilename.c_str(), static_cast<int>(r));
		return false;
	}
	if (file == nullptr) {
		Log().Error(L"Failed to open \"%s\": returned nullptr", wfilename.c_str());
		return false;
	}

	// 80-byte header
	constexpr const char header[] = 
		// 234567890123456789
		"**MeshProc** created"
		" stl file for fun an"
		"d profit............"
		"....................";
	static_assert(sizeof(header) >= 80);
	fwrite(header, 1, 80, file);

	// tri count uint32
	uint32_t triCnt = 0;
	for (auto const& mesh : Scene.Get()->m_meshes)
	{
		triCnt += static_cast<uint32_t>(mesh.first->triangles.size());
	}

	fwrite(&triCnt, 4, 1, file);

	// foreach tri
	uint16_t nullAttr = 0;
	glm::vec3 nullNormal{ 0.0f, 0.0f, 0.0f };
	for (auto const& mesh : Scene.Get()->m_meshes)
	{
		for (Triangle const& t : mesh.first->triangles)
		{
			// 3*float normal
			fwrite(glm::value_ptr(nullNormal), 4, 3, file);

			// 3*3*float vertices
			for (int i = 0; i < 3; ++i) {
				glm::vec4 v = mesh.second * glm::vec4{ mesh.first->vertices[t[i]], 1.0f };
				v *= 1.0f / v.w;
				fwrite(glm::value_ptr(v), 4, 3, file);
			}

			// 16 byte attribute
			fwrite(&nullAttr, 2, 1, file);
		}
	}

	// done.
	fclose(file);
	Log().Detail(L"Written %d triangles to %s", static_cast<int>(triCnt), wfilename.c_str());
	return true;
}
