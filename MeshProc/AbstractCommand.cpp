#include "AbstractCommand.h"

#include <SimpleLog/SimpleLog.hpp>

#include <stdexcept>

using namespace meshproc;

AbstractCommand::AbstractCommand(const sgrottel::ISimpleLog& log)
	: m_log{ log }
	, m_paramsRefs{}
{
}

void AbstractCommand::LogInfo(const sgrottel::ISimpleLog& log, bool verbose) const
{
	m_paramsRefs.LogInfo(log, verbose);
}

void AbstractCommand::InitTypeName(std::string const& name)
{
	if (m_typeName.empty())
	{
		m_typeName = name;
	}
	else
	{
		m_log.Critical("AbstractCommand::InitTypeName must only be called once");
		exit(-1);
	}
}

void AbstractCommand::ParamBindingRefs::LogInfo(const sgrottel::ISimpleLog& log, bool verbose) const
{
	if (!verbose)
	{
		return;
	}
	for (auto const& p : m_params)
	{
		log.Detail("  %s  [%s]  %s", p.first.c_str(), GetParamModeName(p.second->m_mode), GetParamTypeName(p.second->m_type));
	}
}

std::shared_ptr<AbstractCommand::ParamBindingRefs::ParamBindingBase>
AbstractCommand::ParamBindingRefs::GetParam(const std::string& name) const
{
	auto p = m_params.find(name);
	if (p == m_params.end())
	{
		return nullptr;
	}
	return p->second;
}
