#include "PlyReader.h"

#include <SimpleLog/SimpleLog.hpp>

#include <fstream>
#include <string>
#include <string_view>
#include <sstream>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::io;

namespace
{

	int TypeStrToFormatId(const char* name)
	{
		if (strcmp(name, "char") == 0) return 0;
		if (strcmp(name, "uchar") == 0) return 1;
		if (strcmp(name, "short") == 0) return 2;
		if (strcmp(name, "ushort") == 0) return 3;
		if (strcmp(name, "int") == 0) return 4;
		if (strcmp(name, "uint") == 0) return 5;
		if (strcmp(name, "float") == 0) return 6;
		if (strcmp(name, "double") == 0) return 7;
		return -1;
	}

	int FormatByteSize(int id)
	{
		switch (id)
		{
		case 0: return 1;
		case 1: return 1;
		case 2: return 2;
		case 3: return 2;
		case 4: return 4;
		case 5: return 4;
		case 6: return 4;
		case 7: return 8;
		}
		return -1;
	}

	template<typename T>
	T ReadAs(const uint8_t* buf, int offset, int format)
	{
		switch (format)
		{
		case 0: return static_cast<T>(*reinterpret_cast<const int8_t*>(buf + offset));
		case 1: return static_cast<T>(*reinterpret_cast<const uint8_t*>(buf + offset));
		case 2: return static_cast<T>(*reinterpret_cast<const int16_t*>(buf + offset));
		case 3: return static_cast<T>(*reinterpret_cast<const uint16_t*>(buf + offset));
		case 4: return static_cast<T>(*reinterpret_cast<const int32_t*>(buf + offset));
		case 5: return static_cast<T>(*reinterpret_cast<const uint32_t*>(buf + offset));
		case 6: return static_cast<T>(*reinterpret_cast<const float*>(buf + offset));
		case 7: return static_cast<T>(*reinterpret_cast<const double*>(buf + offset));
		}
		return static_cast<T>(0);
	}

	float ReadAsFloat(const uint8_t* buf, int offset, int format)
	{
		return ReadAs<float>(buf, offset, format);
	}


	uint32_t ReadAsUInt32(const uint8_t* buf, int offset, int format)
	{
		return ReadAs<uint32_t>(buf, offset, format);
	}

}

PlyReader::PlyReader(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("Mesh", m_mesh);
}

bool PlyReader::Invoke()
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

	Log().Message(L"Reading PLY: %s", m_path.c_str());

	// header
	constexpr size_t lineBufSize = 1024;
	char lineBuf[lineBufSize + 1];
	lineBuf[lineBufSize] = 0;
	auto ReadLine = [&]() {
		if (fgets(lineBuf, lineBufSize, file) == nullptr)
		{
			Log().Error(L"Failed to read header line");
			fclose(file);
			return false;
		}
		return true;
		};

	if (!ReadLine()) return false;
	if (strcmp(lineBuf, "ply\n") != 0)
	{
		Log().Error(L"Failed to read first header id line 'ply'");
		fclose(file);
		return false;
	}

	int fileFormat = 0;

	int vertCnt = 0;
	int vertSize = 0;
	int vertXOffset = -1;
	int vertXFormat = -1;
	int vertYOffset = -1;
	int vertYFormat = -1;
	int vertZOffset = -1;
	int vertZFormat = -1;

	int faceCnt = 0;
	int faceSize = 0;
	int faceListLenOffset = -1;
	int faceListLenFormat = -1;
	int faceListFormat = -1;

	int curElement = 0;

	while (ReadLine())
	{
		if (strcmp(lineBuf, "end_header\n") == 0)
		{
			curElement = 3;
			break;
		}
		else if (strncmp(lineBuf, "format ", 7) == 0)
		{
			char formatType[256];
			char formatVersion[256];
			if (sscanf_s(lineBuf, "format %255s %255s\n", &formatType, static_cast<unsigned int>(_countof(formatType)), &formatVersion, static_cast<unsigned int>(_countof(formatVersion))) != 2)
			{
				Log().Error(L"Failed to read header format line");
				fclose(file);
				return false;
			}
			formatType[255] = 0;
			formatVersion[255] = 0;
			if (strcmp(formatType, "binary_little_endian") == 0 && strcmp(formatVersion, "1.0") == 0)
			{
				fileFormat = 1;
			}
			else
			{
				Log().Error(L"ERROR: Currently, only PLY format 'binary_little_endian 1.0' is supported");
				fclose(file);
				return false;
			}
		}
		else if (strncmp(lineBuf, "comment ", 8) == 0)
		{
			// ignore comment rest of line
			continue;
		}
		else if (strncmp(lineBuf, "element ", 8) == 0)
		{
			char name[256];
			int size;
			if (sscanf_s(lineBuf, "element %255s %d\n", &name, static_cast<unsigned int>(_countof(name)), &size) != 2)
			{
				Log().Error(L"Failed to read header element line: %s", lineBuf);
				fclose(file);
				return false;
			}
			name[255] = 0;
			if (strcmp(name, "vertex") == 0)
			{
				curElement = 1;
				vertCnt += size;
			}
			else if (strcmp(name, "face") == 0)
			{
				curElement = 2;
				faceCnt += size;
			}
			else
			{
				curElement = -1;
			}
		}
		else if (strncmp(lineBuf, "property ", 9) == 0)
		{
			char name[256];
			char typeStr[256];
			char typeStr2[256];
			bool isList = false;
			int typeFormat = -1;
			int subTypeFormat = -1;

			if (sscanf_s(lineBuf, "property list %255s %255s %255s\n", &typeStr, static_cast<unsigned int>(_countof(typeStr)), &typeStr2, static_cast<unsigned int>(_countof(typeStr2)), &name, static_cast<unsigned int>(_countof(name))) == 3)
			{
				isList = true;
				typeStr2[255] = 0;
				subTypeFormat = TypeStrToFormatId(typeStr2);
				if (subTypeFormat < 0)
				{
					Log().Error(L"Failed to read header property line, unsupported list second type: %s", lineBuf);
					fclose(file);
					return false;
				}
			}
			else if (sscanf_s(lineBuf, "property %255s %255s\n", &typeStr, static_cast<unsigned int>(_countof(typeStr)), &name, static_cast<unsigned int>(_countof(name))) == 2)
			{
				isList = false;
			}
			else
			{
				Log().Error(L"Failed to read header property line: %s", lineBuf);
				fclose(file);
				return false;
			}
			name[255] = 0;
			typeStr[255] = 0;
			typeFormat = TypeStrToFormatId(typeStr);
			if (typeFormat < 0)
			{
				Log().Error(L"Failed to read header property line, unsupported list second type: %s", lineBuf);
				fclose(file);
				return false;
			}

			if (curElement == 1)
			{
				if (isList)
				{
					Log().Error(L"Failed to read header property line, list property in vertex element is not supported: %s", lineBuf);
					fclose(file);
					return false;
				}

				if (strcmp(name, "x") == 0)
				{
					vertXOffset = vertSize;
					vertXFormat = typeFormat;
				}
				else if (strcmp(name, "y") == 0)
				{
					vertYOffset = vertSize;
					vertYFormat = typeFormat;
				}
				else if (strcmp(name, "z") == 0)
				{
					vertZOffset = vertSize;
					vertZFormat = typeFormat;
				}

				vertSize += FormatByteSize(typeFormat);
			}
			else if (curElement == 2)
			{
				if (isList)
				{
					if (strcmp(name, "vertex_indices") != 0)
					{
						Log().Error(L"Failed to read header property list line, 'vertex_indices' list property must be first in face element: %s", lineBuf);
						fclose(file);
						return false;
					}

					faceListLenOffset = faceSize;
					faceListLenFormat = typeFormat;
					faceListFormat = subTypeFormat;

					// assume all triangles
					faceSize += FormatByteSize(faceListLenFormat) + 3 * FormatByteSize(faceListFormat);
				}
				else
				{
					faceSize += FormatByteSize(typeFormat);
				}
			}
			else
			{
				// ignoring custom elements
				continue;
			}
		}
	}
	if (curElement != 3)
	{
		Log().Error(L"Failed to read header: %d", curElement);
		fclose(file);
		return false;
	}
	if (fileFormat != 1)
	{
		Log().Error(L"Failed to read header: unknown file format");
		fclose(file);
		return false;
	}

	if (vertXOffset < 0 || vertXFormat < 0 ||
		vertYOffset < 0 || vertYFormat < 0 ||
		vertZOffset < 0 || vertZFormat < 0)
	{
		Log().Error(L"Failed to read header: vertex position data incomplete or unsupported");
		fclose(file);
		return false;
	}
	if (faceListLenOffset < 0 || faceListLenFormat < 0 || faceListFormat < 0)
	{
		Log().Error(L"Failed to read header: face data incomplete or unsupported");
		fclose(file);
		return false;
	}
	const int faceListOffset = faceListLenOffset + FormatByteSize(faceListLenFormat);
	const int faceListFormatSize = FormatByteSize(faceListFormat);

	std::vector<uint8_t> buf;

	auto mesh = std::make_shared<data::Mesh>();

	mesh->vertices.clear();
	mesh->vertices.resize(vertCnt);

	buf.resize(vertSize);
	for (int i = 0; i < vertCnt; ++i)
	{
		if (fread(buf.data(), 1, vertSize, file) != vertSize)
		{
			Log().Error(L"Failed to read vertex data");
			fclose(file);
			return false;
		}

		auto& v = mesh->vertices[i];
		v.x = ReadAsFloat(buf.data(), vertXOffset, vertXFormat);
		v.y = ReadAsFloat(buf.data(), vertYOffset, vertYFormat);
		v.z = ReadAsFloat(buf.data(), vertZOffset, vertZFormat);
	}

	mesh->triangles.clear();
	mesh->triangles.resize(faceCnt);

	buf.resize(faceSize);
	for (int i = 0; i < faceCnt; ++i)
	{
		if (fread(buf.data(), 1, faceSize, file) != faceSize)
		{
			Log().Error(L"Failed to read face data");
			fclose(file);
			return false;
		}

		uint32_t listLen = ReadAsUInt32(buf.data(), faceListLenOffset, faceListLenFormat);
		if (listLen != 3)
		{
			Log().Error(L"Failed to read face data, only triangle faces are currently supported");
			fclose(file);
			return false;
		}

		auto& t = mesh->triangles[i];
		t[0] = ReadAsUInt32(buf.data(), faceListOffset, faceListFormat);
		t[1] = ReadAsUInt32(buf.data(), faceListOffset + faceListFormatSize, faceListFormat);
		t[2] = ReadAsUInt32(buf.data(), faceListOffset + faceListFormatSize * 2, faceListFormat);
	}

	if (!mesh->IsValid())
	{
		Log().Error("Loaded mesh is not valid");
	}

	m_mesh = mesh;
	return true;
}
