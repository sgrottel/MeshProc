#include "SelectConnectedComponentVertices.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;

edit::SelectConnectedComponentVertices::SelectConnectedComponentVertices(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::InOut, ParamType::IndexList>("Selection", m_selection);
}

bool edit::SelectConnectedComponentVertices::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_selection)
	{
		Log().Error("Selection is empty");
		return false;
	}

	std::unordered_set<uint32_t> sel;
	sel.reserve(m_selection->size());
	for (uint32_t i : *m_selection)
	{
		sel.insert(i);
	}

	std::unordered_set<uint32_t> tris;
	tris.reserve(m_mesh->triangles.size());
	for (size_t i = 0; i < m_mesh->triangles.size(); ++i)
	{
		tris.insert(static_cast<uint32_t>(i));
	}

	std::unordered_set<uint32_t> gt;
	gt.reserve(tris.size() / 100);
	bool grow = true;
	while (grow)
	{
		grow = false;

		for (uint32_t ti : tris)
		{
			const auto tr = m_mesh->triangles.at(ti);

			const bool s0 = sel.contains(tr[0]);
			const bool s1 = sel.contains(tr[1]);
			const bool s2 = sel.contains(tr[2]);
			if (s0 || s1 || s2)
			{
				gt.insert(ti);
				if (!s0)
				{
					sel.insert(tr[0]);
					grow = true;
				}
				if (!s1)
				{
					sel.insert(tr[1]);
					grow = true;
				}
				if (!s2)
				{
					sel.insert(tr[2]);
					grow = true;
				}
			}
		}
		for (uint32_t ti : gt)
		{
			tris.erase(ti);
		}
		gt.clear();
	}

	m_selection->clear();
	m_selection->reserve(sel.size());
	for (uint32_t i : sel)
	{
		m_selection->push_back(i);
	}

	return true;
}
