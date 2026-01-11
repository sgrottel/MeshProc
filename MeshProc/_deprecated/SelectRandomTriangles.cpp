#include "SelectRandomTriangles.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_set>

using namespace meshproc;

SelectRandomTriangles::SelectRandomTriangles(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Float>("AmountRatio", m_amountRatio);
	AddParamBinding<ParamMode::Out, ParamType::Indices>("Triangles", m_tris);
	AddParamBinding<ParamMode::In, ParamType::UInt32>("Seed", m_seed);
}

bool SelectRandomTriangles::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}

	const float amount = std::clamp(m_amountRatio, 0.0f, 1.0f);
	Log().Detail("Selecting %f triangles from mesh", amount);

	const size_t cnt = static_cast<size_t>(m_mesh->triangles.size() * amount);
	Log().Detail("Selecting %d triangles from mesh", static_cast<int>(cnt));

	Log().Detail("Random Seed: %u", static_cast<unsigned int>(m_seed));

	std::mt19937 gen(m_seed);
	std::uniform_int_distribution<uint32_t> random(0, static_cast<uint32_t>(m_mesh->triangles.size() - 1));

	std::unordered_set<uint32_t> sel;
	sel.reserve(cnt);
	for (size_t i = 0; i < cnt; ++i)
	{
		uint32_t n = 0;
		do {
			n = random(gen);
		} while (sel.contains(n));
		sel.insert(n);
	}

	m_tris = std::make_shared<std::vector<uint32_t>>(cnt);
	std::copy(sel.begin(), sel.end(), m_tris->begin());

	return true;
}
