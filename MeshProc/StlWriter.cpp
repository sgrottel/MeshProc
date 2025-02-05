#include "StlWriter.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <cstdio>

StlWriter::StlWriter(sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

void StlWriter::Save(std::wstring const& filename, std::shared_ptr<Mesh> mesh)
{
	FILE* file = nullptr;
	errno_t r = _wfopen_s(&file, filename.c_str(), L"wb");
	if (r != 0) {
		m_log.Error(L"Failed to open \"%s\": %d", filename.c_str(), static_cast<int>(r));
		return;
	}
	if (file == nullptr) {
		m_log.Error(L"Failed to open \"%s\": returned nullptr", filename.c_str());
		return;
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
	uint32_t triCnt = mesh->triangles.size();
	fwrite(&triCnt, 4, 1, file);

	// foreach tri
	uint16_t nullAttr = 0;
	glm::vec3 nullNormal{ 0.0f, 0.0f, 0.0f };
	for (glm::uvec3 const& t : mesh->triangles)
	{
		// 3*float normal
		fwrite(glm::value_ptr(nullNormal), 4, 3, file);

		// 3*3*float vertices
		for (int i = 0; i < 3; ++i) {
			fwrite(glm::value_ptr(mesh->vertices[t[i]]), 4, 3, file);
		}

		// 16 byte attribute
		fwrite(&nullAttr, 2, 1, file);
	}

	// done.
	fclose(file);
	m_log.Detail(L"Written %d triangles to %s", static_cast<int>(mesh->triangles.size()), filename.c_str());
}
