#include "StlReader.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cstdio>
#include <unordered_map>

namespace
{
	#pragma pack(1)
	struct StlTriData
	{
		float xn, yn, zn;
		union {
			struct {
				float x1, y1, z1;
				float x2, y2, z2;
				float x3, y3, z3;
			};
			glm::vec3 v[3];
		};
		uint16_t attr;
	};

	static_assert(sizeof(StlTriData) == 50);

}

StlReader::StlReader(sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

std::shared_ptr<Mesh> StlReader::Load(std::filesystem::path const& filename)
{
	std::wstring wfilename{ filename.wstring() };
	FILE* file = nullptr;
	errno_t r = _wfopen_s(&file, wfilename.c_str(), L"rb");
	if (r != 0) {
		m_log.Error(L"Failed to open \"%s\": %d", wfilename.c_str(), static_cast<int>(r));
		return nullptr;
	}
	if (file == nullptr) {
		m_log.Error(L"Failed to open \"%s\": returned nullptr", wfilename.c_str());
		return nullptr;
	}

	m_log.Message(L"Reading STL: %s", wfilename.c_str());

	char header[80];
	if (fread(header, 80, 1, file) != 1)
	{
		m_log.Error(L"Failed read file format header. Truncated?");
		return nullptr;
	}
	if (memcmp(header, "solid", 5) == 0)
	{
		m_log.Error(L"File appeards to be ASCII STL, which is not supported.");
		return nullptr;
	}

	uint32_t numTri = 0;
	if (fread(&numTri, 4, 1, file) != 1)
	{
		m_log.Error(L"Failed read data. Truncated?");
		return nullptr;
	}

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
	mesh->vertices.reserve(numTri * 2);
	mesh->triangles.reserve(numTri);

	std::vector<StlTriData> buf;
	buf.resize(1000);

	std::unordered_map<glm::vec3, uint32_t> vertexIndex;
	uint32_t idx;

	while (numTri > 0)
	{
		size_t readCnt = fread(buf.data(), sizeof(StlTriData), std::min<size_t>(numTri, buf.size()), file);
		for (size_t i = 0; i < readCnt; ++i)
		{
			StlTriData const& t = buf[i];
			assert(t.v[0].x == t.x1);
			assert(t.v[0].y == t.y1);
			assert(t.v[0].z == t.z1);
			assert(t.v[1].x == t.x2);
			assert(t.v[1].y == t.y2);
			assert(t.v[1].z == t.z2);
			assert(t.v[2].x == t.x3);
			assert(t.v[2].y == t.y3);
			assert(t.v[2].z == t.z3);

			mesh->triangles.push_back({});

			for (int j = 0; j < 3; ++j)
			{
				auto const it = vertexIndex.find(t.v[j]);
				if (it == vertexIndex.end())
				{
					idx = static_cast<uint32_t>(mesh->vertices.size());
					vertexIndex.insert({ t.v[j], idx });
					mesh->vertices.push_back(t.v[j]);
				}
				else
				{
					idx = it->second;
					assert(t.v[j] == mesh->vertices[idx]);
				}
				mesh->triangles.back()[j] = idx;
			}
		}

		numTri -= std::min<uint32_t>(numTri, static_cast<uint32_t>(readCnt));
	}

	fclose(file);

	mesh->vertices.shrink_to_fit();

	if (!mesh->IsValid())
	{
		m_log.Error("Loaded mesh is not valid");
	}

	return mesh;
}
