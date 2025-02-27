#include "AbstractCommand.h"

#include <SimpleLog/SimpleLog.hpp>

#include <stdexcept>

using namespace meshproc;

AbstractCommand::AbstractCommand(const sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

void AbstractCommand::LogInfo(const sgrottel::ISimpleLog& log, bool verbose) const
{
	for (auto const& p : m_params)
	{
		log.Detail("  %s  [%s]  %s", p.first.c_str(), GetParamModeName(p.second->m_mode), GetParamTypeName(p.second->m_type));
	}
}

std::shared_ptr<AbstractCommand::ParamBindingBase> AbstractCommand::AccessParam(const std::string& name) const
{
	auto p = m_params.find(name);
	if (p == m_params.end())
	{
		return nullptr;
	}
	return p->second;
}
