#include "CommandFactory.h"

#include "AbstractCommand.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

CommandFactory::CommandFactory(const sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

void CommandFactory::ListCommands(bool verbose) const
{
	sgrottel::NullLog nullLog;

	for (auto const& ct : m_commandTemplates)
	{
		m_log.Message("%s", ct.first.c_str());
		std::shared_ptr<AbstractCommand> cmd = ct.second->Instantiate(nullLog);
		cmd->LogInfo(m_log, verbose);
	}
	m_log.Message("");
	if (!verbose)
	{
		m_log.Message("Switch to verbose output for more information.");
	}
}

std::shared_ptr<class AbstractCommand> CommandFactory::Instantiate(const std::string& name, const sgrottel::ISimpleLog& log) const
{
	auto t = m_commandTemplates.find(name);
	if (t == m_commandTemplates.end())
	{
		m_log.Error("Unable to instantiate command \"%s\": not found", name.c_str());
		return nullptr;
	}
	return t->second->Instantiate(log);
}

void CommandFactory::Log(const char* msg, const char* a)
{
	m_log.Error(msg, a);
}
