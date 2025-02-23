#include "AbstractCommand.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

AbstractCommand::AbstractCommand(const sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

void AbstractCommand::PreInvoke()
{
	for (const auto& p : m_params)
	{
		std::get<0>(p)->PreInvoke();
	}
}

void AbstractCommand::PostInvoke()
{
	for (const auto& p : m_params)
	{
		std::get<0>(p)->PostInvoke();
	}
}

AbstractCommand& AbstractCommand::AddParam(const char* name, ParameterBase& param)
{
	m_params.push_back(std::make_tuple<ParameterBase*, std::string>(&param, name));
	m_log.Detail("Added param \"%s\"", name);
	return *this;
}
