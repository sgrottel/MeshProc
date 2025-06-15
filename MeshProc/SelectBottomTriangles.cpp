#include "SelectBottomTriangles.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

SelectBottomTriangles::SelectBottomTriangles(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::Out, ParamType::VertexSelection>("TrianglesList", m_triList);
}

bool SelectBottomTriangles::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (m_mesh->triangles.empty())
	{
		Log().Error("Mesh has no triangles");
		return false;
	}
	if (m_mesh->vertices.empty())
	{
		Log().Error("Mesh has no vertices");
		return false;
	}

	Log().Detail("Detecting bottom faces");

	float minZ = m_mesh->vertices[0].z;
	for (const auto& v : m_mesh->vertices)
	{
		if (v.z < minZ) minZ = v.z;
	}

	constexpr float zEps = 0.01f;

	std::vector<bool> vsel;
	vsel.resize(m_mesh->vertices.size());
	std::transform(
		m_mesh->vertices.begin(),
		m_mesh->vertices.end(),
		vsel.begin(),
		[z = minZ + zEps](const glm::vec3& v)
		{
			bool sel = v.z < z;
			return sel;
		});

	size_t numSel = std::count_if(vsel.begin(), vsel.end(), [](const bool& b) { return b; });
	Log().Detail("Selected %d vertices at bottom", static_cast<int>(numSel));

	std::shared_ptr<std::vector<uint32_t>> triList = std::make_shared<std::vector<uint32_t>>();

	for (size_t i = 0; i < m_mesh->triangles.size(); ++i)
	{
		const auto& t = m_mesh->triangles[i];
		if (vsel[t[0]] && vsel[t[1]] && vsel[t[2]])
		{
			triList->push_back(static_cast<uint32_t>(i));
		}
	}

	Log().Detail("Selected %d triangles at bottom", static_cast<int>(triList->size()));

	m_triList = triList;
	return true;
}
