#include "VertexColorWriter.h"

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

namespace
{
	struct FileCloser
	{
		void operator()(FILE *f)
		{
			fclose(f);
		}
	};
}

VertexColorWriter::VertexColorWriter(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::In, ParamType::Vec3List>("Colors", m_colors);
}

bool VertexColorWriter::Invoke()
{
	if (!m_colors)
	{
		Log().Critical("Colors not set");
		return false;
	}

	std::unique_ptr<FILE, FileCloser> file;
	{
		FILE *raw = nullptr;
		errno_t r = _wfopen_s(&raw, m_path.c_str(), L"wb");
		if (r != 0) {
			Log().Error(L"Failed to open \"%s\": %d", m_path.c_str(), static_cast<int>(r));
			return false;
		}
		if (raw == nullptr) {
			Log().Error(L"Failed to open \"%s\": returned nullptr", m_path.c_str());
			return false;
		}
		file.reset(raw);
	}

	Log().Message(L"Writing vertex color file: %s", m_path.c_str());


	// Magic
	if (fwrite("SGRVCOL\0", 1, 8, file.get()) != 8)
	{
		throw std::runtime_error("Failed to write header id");
	}

	// Version
	const uint16_t version = 1;
	if (fwrite(&version, 2, 1, file.get()) != 1)
	{
		throw std::runtime_error("Failed to write header version");
	}

	// Vertex count
	const uint32_t vertexCount = static_cast<uint32_t>(m_colors->size());
	if (fwrite(&vertexCount, 4, 1, file.get()) != 1)
	{
		throw std::runtime_error("Failed to write");
	}

	// Channel count
	uint8_t channels = 3;
	if (fwrite(&channels, 1, 1, file.get()) != 1)
	{
		throw std::runtime_error("Failed to write");
	}

	// Color data
	static_assert(sizeof(float) == 4);
	static_assert(sizeof(glm::vec3) == 3 * sizeof(float));
	static_assert(offsetof(glm::vec3, x) == 0);
	static_assert(offsetof(glm::vec3, y) == sizeof(float));
	static_assert(offsetof(glm::vec3, z) == 2 * sizeof(float));
	if (fwrite(m_colors->data(), sizeof(glm::vec3), vertexCount, file.get()) != vertexCount)
	{
		throw std::runtime_error("Failed to write");
	}

	// Footer
	const uint16_t footer = 0xFF00;
	if (fwrite(&footer, 2, 1, file.get()) != 1)
	{
		throw std::runtime_error("Failed to write file footer");
	}

	file.reset();

	return true;
}
