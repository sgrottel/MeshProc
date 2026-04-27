#include "InvertVertexSelection.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;

edit::InvertVertexSelection::InvertVertexSelection(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::InOut, ParamType::IndexList>("Selection", m_selection);
}

bool edit::InvertVertexSelection::Invoke()
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

	std::sort(m_selection->begin(), m_selection->end());

	std::vector<uint32_t> comp;
	comp.reserve(m_mesh->vertices.size() - m_selection->size());

	size_t selIdx = 0;
	for (uint32_t i = 0; i < m_mesh->vertices.size(); ++i)
	{
		while (selIdx < m_selection->size() && m_selection->at(selIdx) < i)
		{
			++selIdx;
		}
		if (selIdx < m_selection->size() && m_selection->at(selIdx) == i)
		{
			++selIdx;
		}
		else
		{
			comp.push_back(i);
		}
	}

	std::swap(comp, *m_selection);

	return true;
}
