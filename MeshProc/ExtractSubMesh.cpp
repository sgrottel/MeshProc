#include "ExtractSubMesh.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_map>

using namespace meshproc;

ExtractSubMesh::ExtractSubMesh(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("InputMesh", m_inputMesh);
	AddParamBinding<ParamMode::In, ParamType::VertexSelection>("TrianglesList", m_triList);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("OutputMesh", m_outputMesh);
}

bool ExtractSubMesh::Invoke()
{
	if (!m_inputMesh)
	{
		Log().Error("InputMesh is empty");
		return false;
	}
	if (!m_triList)
	{
		Log().Error("TrianglesList is empty");
		return false;
	}

	std::unordered_map<uint32_t, uint32_t> vMap;
	for (uint32_t ti : *m_triList)
	{
		if (ti >= m_inputMesh->triangles.size())
		{
			Log().Error("TrianglesList reference out of bounds in InputMesh");
			return false;
		}

		const auto& t = m_inputMesh->triangles[ti];
		for (int i = 0; i < 3; ++i)
		{
			if (!vMap.contains(t[i]))
			{
				vMap[t[i]] = vMap.size();
			}
		}
	}

	auto mesh = std::make_shared<data::Mesh>();

	mesh->vertices.resize(vMap.size());
	for (const auto& vm : vMap)
	{
		mesh->vertices[vm.second] = m_inputMesh->vertices[vm.first];
	}

	mesh->triangles.resize(m_triList->size());
	std::transform(
		m_triList->begin(),
		m_triList->end(),
		mesh->triangles.begin(),
		[&](uint32_t ti)
		{
			data::Triangle tri;
			const auto& t = m_inputMesh->triangles[ti];
			for (int i = 0; i < 3; ++i)
			{
				tri[i] = vMap[t[i]];
			}
			return tri;
		}
	);

	m_outputMesh = mesh;
	return true;
}
