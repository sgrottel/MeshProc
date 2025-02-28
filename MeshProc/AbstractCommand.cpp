#include "AbstractCommand.h"

#include "ParameterValue.h"

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

bool AbstractCommand::PullParamValue(ParameterValue& target, const std::string& name) const
{
	auto p = m_paramsRefs.GetParam(name);
	if (!p)
	{
		// not found
		return false;
	}
	if (p->m_mode != ParamMode::InOut && p->m_mode != ParamMode::Out)
	{
		// not readable
		return false;
	}

	if (!target.Push(*p))
	{
		// failed to push for unknown reason
		return false;
	}

	return true;
}

bool AbstractCommand::PushParamValue(const std::string& name, const ParameterValue& source)
{
	auto p = m_paramsRefs.GetParam(name);
	if (!p)
	{
		// not found
		return false;
	}
	if (p->m_mode != ParamMode::InOut && p->m_mode != ParamMode::In)
	{
		// not writable
		return false;
	}

	if (!source.Pull(*p))
	{
		// failed to push for unknown reason
		return false;
	}

	return true;
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
