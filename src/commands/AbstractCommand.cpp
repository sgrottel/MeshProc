#include "AbstractCommand.h"

#include <SimpleLog/SimpleLog.hpp>

#include <stdexcept>

using namespace meshproc;
using namespace meshproc::commands;

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
	std::vector<std::pair<std::string, std::shared_ptr<ParamBindingBase>>> params;
	params.resize(m_params.size());
	std::copy(m_params.begin(), m_params.end(), params.begin());
	std::sort(params.begin(), params.end(), [](auto& a, auto& b) { return std::get<1>(a)->m_idx < std::get<1>(b)->m_idx; });
	for (auto const& p : params)
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
