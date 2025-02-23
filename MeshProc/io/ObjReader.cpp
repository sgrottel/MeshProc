#include "ObjReader.h"

#include <SimpleLog/SimpleLog.hpp>

#include <fstream>
#include <string>
#include <string_view>
#include <sstream>

using namespace meshproc;
using namespace meshproc::io;

namespace
{

	__forceinline bool ParseVertexFloat(std::string_view::const_iterator& i, std::string_view::const_iterator end, float& v)
	{
		while (i != end && std::isspace(*i)) i++;
		std::string_view::const_iterator b{ i };
		while (i != end && (std::isdigit(*i) || *i == '.' || *i == '-')) i++;
		if (b != i)
		{
			std::string s{ b, i };
			v = static_cast<float>(std::atof(s.c_str()));
			return true;
		}
		return false;
	}

	__forceinline bool ParseFaceUInt(std::string_view::const_iterator& i, std::string_view::const_iterator end, uint32_t& v)
	{
		while (i != end && std::isspace(*i)) i++;
		std::string_view::const_iterator b{ i };
		while (i != end && std::isdigit(*i)) i++;
		std::string_view::const_iterator be{ i };
		while (i != end && !std::isspace(*i)) i++;

		if (b != be)
		{
			std::string s{ b, be };
			v = static_cast<uint32_t>(std::atoi(s.c_str()));
			return true;
		}
		return false;
	}

}


ObjReader::ObjReader(sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
}

bool ObjReader::Invoke()
{
	std::ifstream file{ Path.Get(), std::ios::in };
	if (!file.is_open())
	{
		Log().Error("Failed to open file");
		return false;
	}

	Log().Message(L"Reading OBJ: %s", std::wstring{Path.Get()}.c_str());

	auto mesh = std::make_shared<data::Mesh>();

	std::string lineBuf;
	while (std::getline(file, lineBuf))
	{
		std::string_view line{ lineBuf };
		while (!line.empty() && std::isspace(line.front()))
		{
			line = line.substr(1);
		}
		if (line.empty()) continue;
		if (line.front() == '#') continue;

		if (line.starts_with("v "))
		{
			std::string_view::const_iterator i = line.begin() + 2;
			std::string_view::const_iterator e = line.end();
			float x, y, z;
			if (!ParseVertexFloat(i, e, x))
			{
				Log().Error("Failed to parse vertex float");
				continue;
			}
			if (!ParseVertexFloat(i, e, y))
			{
				Log().Error("Failed to parse vertex float");
				continue;
			}
			if (!ParseVertexFloat(i, e, z))
			{
				Log().Error("Failed to parse vertex float");
				continue;
			}

			mesh->vertices.push_back({ x, y, z });
		}

		if (line.starts_with("f "))
		{
			std::string_view::const_iterator i = line.begin() + 2;
			std::string_view::const_iterator e = line.end();
			uint32_t v0, v1, v2;
			if (!ParseFaceUInt(i, e, v0))
			{
				Log().Error("Failed to parse vertex float");
				continue;
			}
			if (!ParseFaceUInt(i, e, v1))
			{
				Log().Error("Failed to parse vertex float");
				continue;
			}
			if (!ParseFaceUInt(i, e, v2))
			{
				Log().Error("Failed to parse vertex float");
				continue;
			}

			mesh->triangles.push_back({ v0 - 1, v1 - 1, v2 - 1 });
		}

	}

	mesh->vertices.shrink_to_fit();
	mesh->triangles.shrink_to_fit();

	if (!mesh->IsValid())
	{
		Log().Error("Loaded mesh is not valid");
	}

	Mesh.Put() = mesh;
	return true;
}
