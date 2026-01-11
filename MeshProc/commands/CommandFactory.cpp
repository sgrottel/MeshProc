#include "CommandFactory.h"

#include "AbstractCommand.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;

CommandFactory::CommandFactory(const sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

void CommandFactory::ListCommands(bool verbose) const
{
	sgrottel::NullLog nullLog;

	for (auto const& name : GetAllNames())
	{
		m_log.Message("%s", name.c_str());
		std::shared_ptr<AbstractCommand> cmd = m_commandTemplates.at(name)->Instantiate(nullLog);
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
	std::shared_ptr<class AbstractCommand> cmd = t->second->Instantiate(log);
	cmd->InitTypeName(t->first);
	return cmd;
}

std::vector<std::string> CommandFactory::GetAllNames() const
{
	std::vector<std::string> names;
	names.reserve(m_commandTemplates.size());
	for (auto const& ct : m_commandTemplates)
	{
		if (ct.second->IsHidden()) continue; // hides from `meshproc` lua alias', still works via `meshproc._createCommand`
		names.push_back(ct.first);
	}
	std::sort(names.begin(), names.end());
	return names;
}

void CommandFactory::HideCommand(const std::string& name)
{
	auto t = m_commandTemplates.find(name);
	if (t != m_commandTemplates.end())
	{
		t->second->SetHide(true);
	}
}

void CommandFactory::Log(const char* msg, const char* a)
{
	m_log.Error(msg, a);
}
