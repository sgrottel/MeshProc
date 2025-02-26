#include "AbstractCommand.h"

#include <SimpleLog/SimpleLog.hpp>

#include <stdexcept>

using namespace meshproc;

AbstractCommand::AbstractCommand(const sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

void AbstractCommand::PreInvoke()
{
	for (const auto& p : m_params)
	{
		p.second->PreInvoke();
	}
}

void AbstractCommand::PostInvoke()
{
	for (const auto& p : m_params)
	{
		p.second->PostInvoke();
	}
}

void AbstractCommand::LogInfo(const sgrottel::ISimpleLog& log, bool verbose) const
{
	for (auto const& p : m_params)
	{
		auto const& pb = p.second;
		log.Detail("  %s  [%s]  %s", p.first.c_str(), pb->ModeStr(), pb->TypeStr());
	}
}

ParameterBase* AbstractCommand::AccessParam(const std::string& name) const
{
	auto p = m_params.find(name);
	if (p == m_params.end())
	{
		return nullptr;
	}
	return p->second;
}

AbstractCommand& AbstractCommand::AddParam(const std::string& name, ParameterBase& param)
{
	if (m_params.find(name) != m_params.end())
	{
		m_log.Critical("A parameter with the name \"%s\" was already added.", name);
		throw std::logic_error("Parameter name conflict");
	}

	m_params[name] = &param;
	m_log.Detail("Added param \"%s\"", name);
	return *this;
}
