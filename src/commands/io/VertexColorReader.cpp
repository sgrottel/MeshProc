#include "VertexColorReader.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::io;

// ─── .vcol Binary File Format ────────────────────────────────────────────────
//
// Offset  Size        Description
// ──────  ──────────  ─────────────────────────────────────────────────────────
// 0       8 bytes     Magic: ASCII "SGRVCOL" followed by a zero byte (0x00)
// 8       2 bytes     Version number (uint16_t): currently 1
// 10      4 bytes     Vertex count N (uint32_t)
// 14      1 byte      Channel count: value 3 (RGB)
// 15      12*N bytes  Color data: 3*N float values in RGBRGBRGB... order
// 15+12N  2 bytes     Footer marker (uint16_t): 0xFF00
//
// All multi-byte values are little-endian.
// ─────────────────────────────────────────────────────────────────────────────

VertexColorReader::VertexColorReader(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::Out, ParamType::Vec3List>("Colors", m_colors);
}

bool VertexColorReader::Invoke()
{
	FILE* file = nullptr;
	errno_t r = _wfopen_s(&file, m_path.c_str(), L"rb");
	if (r != 0) {
		Log().Error(L"Failed to open \"%s\": %d", m_path.c_str(), static_cast<int>(r));
		return false;
	}
	if (file == nullptr) {
		Log().Error(L"Failed to open \"%s\": returned nullptr", m_path.c_str());
		return false;
	}

	Log().Message(L"Reading vertex color file: %s", m_path.c_str());


	// Magic
	char magic[8] = {};
	if (fread(magic, 1, 8, file) != 8 || memcmp(magic, "SGRVCOL", 8) != 0)
	{
		Log().Error("File format header id wrong");
		fclose(file);
		return false;
	}

	// Version
	uint16_t version = 0;
	if (fread(&version, sizeof(version), 1, file) != 1 || version != 1)
	{
		Log().Error("File format version wrong");
		fclose(file);
		return false;
	}

	// Vertex count
	uint32_t vertexCount = 0;
	if (fread(&vertexCount, sizeof(vertexCount), 1, file) != 1) {
		Log().Error("Failed to read file vertex count");
		fclose(file);
		return false;
	}

	// Channel count
	uint8_t channels = 0;
	if (fread(&channels, sizeof(channels), 1, file) != 1 || channels != 3)
	{
		Log().Error("File format channels format wrong");
		fclose(file);
		return false;
	}

	// Color data
	m_colors = std::make_shared<std::vector<glm::vec3>>(static_cast<size_t>(vertexCount), glm::vec3{ 0.0f });
	static_assert(sizeof(glm::vec3) == 3 * sizeof(float));
	static_assert(offsetof(glm::vec3, x) == 0);
	static_assert(offsetof(glm::vec3, y) == sizeof(float));
	static_assert(offsetof(glm::vec3, z) == 2 * sizeof(float));
	if (fread(m_colors->data(), sizeof(glm::vec3), vertexCount, file) != vertexCount)
	{
		Log().Error("Failed to read vertex color data");
		fclose(file);
		return false;
	}

	// Footer
	uint16_t footer = 0;
	if (fread(&footer, sizeof(footer), 1, file) != 1 || footer != 0xFF00)
	{
		Log().Error("File format footer id wrong");
		fclose(file);
		return false;
	}

	fclose(file);

	return true;
}
