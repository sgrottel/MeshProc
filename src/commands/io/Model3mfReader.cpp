#include "Model3mfReader.h"

#include <SimpleLog/SimpleLog.hpp>

#include <lib3mf_implicit.hpp>

#include <iostream>

// #include <fstream>
// #include <string>
// #include <string_view>
// #include <sstream>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::io;

// namespace
// {

// 	__forceinline bool ParseVertexFloat(std::string_view::const_iterator& i, std::string_view::const_iterator end, float& v)
// 	{
// 		while (i != end && std::isspace(*i)) i++;
// 		std::string_view::const_iterator b{ i };
// 		while (i != end && (std::isdigit(*i) || *i == '.' || *i == '-')) i++;
// 		if (b != i)
// 		{
// 			std::string s{ b, i };
// 			v = static_cast<float>(std::atof(s.c_str()));
// 			return true;
// 		}
// 		return false;
// 	}

// 	__forceinline bool ParseFaceUInt(std::string_view::const_iterator& i, std::string_view::const_iterator end, uint32_t& v)
// 	{
// 		while (i != end && std::isspace(*i)) i++;
// 		std::string_view::const_iterator b{ i };
// 		while (i != end && std::isdigit(*i)) i++;
// 		std::string_view::const_iterator be{ i };
// 		while (i != end && !std::isspace(*i)) i++;

// 		if (b != be)
// 		{
// 			std::string s{ b, be };
// 			v = static_cast<uint32_t>(std::atoi(s.c_str()));
// 			return true;
// 		}
// 		return false;
// 	}

// }


Model3mfReader::Model3mfReader(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("Mesh", m_mesh);
}

bool Model3mfReader::Invoke()
{
    // see also:
    // https://github.com/3MFConsortium/lib3mf/blob/master/SDK/Examples/CppDynamic/Source/ExtractInfo.cpp
    //
    Lib3MF::PWrapper wrapper = Lib3MF::CWrapper::loadLibrary();
    Lib3MF::PModel model = wrapper->CreateModel();

    Lib3MF::PReader reader = model->QueryReader("3mf");
    reader->ReadFromFile(m_path);

    // Iterate objects
    for (auto object : model->GetObjects()) {
        std::cout << "Object: " << object->GetName() << "\n";

        if (object->IsMeshObject()) {
            auto mesh = object->GetMeshObject();
            std::cout << "Vertices: " << mesh->GetVertexCount() << "\n";
            std::cout << "Triangles: " << mesh->GetTriangleCount() << "\n";
        }
    }

    return false;

	// std::ifstream file{ m_path, std::ios::in };
	// if (!file.is_open())
	// {
	// 	Log().Error("Failed to open file");
	// 	return false;
	// }

	// Log().Message(L"Reading OBJ: %s", m_path.c_str());

	// auto mesh = std::make_shared<data::Mesh>();

	// std::string lineBuf;
	// while (std::getline(file, lineBuf))
	// {
	// 	std::string_view line{ lineBuf };
	// 	while (!line.empty() && std::isspace(line.front()))
	// 	{
	// 		line = line.substr(1);
	// 	}
	// 	if (line.empty()) continue;
	// 	if (line.front() == '#') continue;

	// 	if (line.starts_with("v "))
	// 	{
	// 		std::string_view::const_iterator i = line.begin() + 2;
	// 		std::string_view::const_iterator e = line.end();
	// 		float x, y, z;
	// 		if (!ParseVertexFloat(i, e, x))
	// 		{
	// 			Log().Error("Failed to parse vertex float");
	// 			continue;
	// 		}
	// 		if (!ParseVertexFloat(i, e, y))
	// 		{
	// 			Log().Error("Failed to parse vertex float");
	// 			continue;
	// 		}
	// 		if (!ParseVertexFloat(i, e, z))
	// 		{
	// 			Log().Error("Failed to parse vertex float");
	// 			continue;
	// 		}

	// 		mesh->vertices.push_back({ x, y, z });
	// 	}

	// 	if (line.starts_with("f "))
	// 	{
	// 		std::string_view::const_iterator i = line.begin() + 2;
	// 		std::string_view::const_iterator e = line.end();
	// 		uint32_t v0, v1, v2;
	// 		if (!ParseFaceUInt(i, e, v0))
	// 		{
	// 			Log().Error("Failed to parse vertex float");
	// 			continue;
	// 		}
	// 		if (!ParseFaceUInt(i, e, v1))
	// 		{
	// 			Log().Error("Failed to parse vertex float");
	// 			continue;
	// 		}
	// 		if (!ParseFaceUInt(i, e, v2))
	// 		{
	// 			Log().Error("Failed to parse vertex float");
	// 			continue;
	// 		}

	// 		mesh->triangles.push_back({ v0 - 1, v1 - 1, v2 - 1 });
	// 	}

	// }

	// mesh->vertices.shrink_to_fit();
	// mesh->triangles.shrink_to_fit();

	// if (!mesh->IsValid())
	// {
	// 	Log().Error("Loaded mesh is not valid");
	// }

	// m_mesh = mesh;
	// return true;
}
