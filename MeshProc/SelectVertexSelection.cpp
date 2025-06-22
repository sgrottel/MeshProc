#include "SelectVertexSelection.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

SelectVertexSelection::SelectVertexSelection(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::UInt32>("Index", m_index);
	AddParamBinding<ParamMode::In, ParamType::MultiVertexSelection>("Lists", m_lists);
	AddParamBinding<ParamMode::Out, ParamType::VertexSelection>("List", m_list);
}

bool SelectVertexSelection::Invoke()
{
	m_list = m_lists->at(m_index);

	return true;
}
