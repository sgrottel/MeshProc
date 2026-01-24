#include "IndicesBoolean.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_set>

using namespace meshproc;

IndicesBoolean::IndicesBoolean(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Indices>("A", m_a);
	AddParamBinding<ParamMode::In, ParamType::Indices>("B", m_b);

	// should be an enum:
	//  0 :  union
	//  1 :  intersection
	//  2 :  A - B
	AddParamBinding<ParamMode::In, ParamType::UInt32>("Operation", m_op);
	AddParamBinding<ParamMode::Out, ParamType::Indices>("Result", m_res);
}

bool IndicesBoolean::Invoke()
{
	if (!m_a)
	{
		Log().Error("Operand A is empty");
		return false;
	}
	if (!m_b)
	{
		Log().Error("Operand B is empty");
		return false;
	}
	if (m_op >= 3)
	{
		Log().Error("Operator value is invalid");
		return false;
	}

	std::unordered_set<uint32_t> a;
	a.insert(m_a->begin(), m_a->end());

	switch (m_op)
	{
	case 0: // union
	{
		a.insert(m_b->begin(), m_b->end());
	}
	break;
	case 1: // intersection
	{
		std::unordered_set<uint32_t> both;
		for (uint32_t i : *m_b)
		{
			if (a.contains(i))
			{
				both.insert(i);
			}
		}
		std::swap(a, both);
	}
	break;
	case 2: // A - B
	{
		for (uint32_t i : *m_b)
		{
			a.erase(i);
		}
	}
	break;
	}

	m_res = std::make_shared<std::vector<uint32_t>>();
	m_res->resize(a.size());
	std::copy(a.begin(), a.end(), m_res->begin());

	return true;
}
