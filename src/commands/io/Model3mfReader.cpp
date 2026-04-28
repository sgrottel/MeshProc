#include "Model3mfReader.h"

#include <SimpleLog/SimpleLog.hpp>

#include <lib3mf_implicit.hpp>

#include <filesystem>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::io;

Model3mfReader::Model3mfReader(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("Mesh", m_mesh);
}

bool Model3mfReader::Invoke()
{
	// only loading via a global file name seem to work, so...
	if (m_path.empty())
	{
		Log().Error("Path not set");
		return false;
	}
	std::filesystem::path filePath = std::filesystem::absolute(m_path);
	if (!std::filesystem::is_regular_file(filePath))
	{
		Log().Error("Path not set to a normal file");
		return false;
	}

	// see also:
	// https://github.com/3MFConsortium/lib3mf/blob/master/SDK/Examples/CppDynamic/Source/ExtractInfo.cpp
	//
	Lib3MF::PWrapper wrapper = Lib3MF::CWrapper::loadLibrary();
	Lib3MF::PModel model = wrapper->CreateModel();

	Lib3MF::PReader reader = model->QueryReader("3mf");

	Log().Message(L"Reading 3MF: %s", filePath.generic_wstring().c_str());

	reader->ReadFromFile(std::string(reinterpret_cast<const char*>(filePath.generic_u8string().c_str())));

	auto mesh = std::make_shared<data::Mesh>();

	int meshCnt = 0;
	auto objI = model->GetObjects();
	while (objI->MoveNext())
	{
		const auto object = objI->GetCurrentObject();
		if (object->IsMeshObject())
		{
			std::string objName = object->GetName();
			Log().Detail("  reading object %d \"%s\"", ++meshCnt, objName.c_str());

			const Lib3MF::PMeshObject meshObj = model->GetMeshObjectByID(object->GetResourceID());

			uint32_t vOff = static_cast<uint32_t>(mesh->vertices.size());
			uint32_t tOff = static_cast<uint32_t>(mesh->triangles.size());

			uint32_t vCnt = meshObj->GetVertexCount();
			mesh->vertices.resize(vOff + vCnt);
			for (uint32_t i = 0; i < vCnt; ++i)
			{
				const auto pos = meshObj->GetVertex(i);
				mesh->vertices.at(vOff + i) = glm::vec3{ pos.m_Coordinates[0], pos.m_Coordinates[1], pos.m_Coordinates[2] };
			}

			uint32_t tCnt = meshObj->GetTriangleCount();
			mesh->triangles.resize(tOff + tCnt);
			for (uint32_t i = 0; i < tCnt; ++i)
			{
				const auto tri = meshObj->GetTriangle(i);
				mesh->triangles.at(tOff + i) = data::Triangle{
					tri.m_Indices[0] + vOff,
					tri.m_Indices[1] + vOff,
					tri.m_Indices[2] + vOff
				};
			}
		}
	}

	if (!mesh->IsValid())
	{
		Log().Error("Loaded mesh is not valid");
	}

	m_mesh = mesh;
	return true;
}
