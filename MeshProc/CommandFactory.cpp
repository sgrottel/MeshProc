#include "CommandFactory.h"

#include "AbstractCommand.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

sgrottel::ISimpleLog& CommandFactory::MyNullLog()
{
	static sgrottel::NullLog nullLog;
	return nullLog;
}

CommandFactory::CommandFactory(const sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

void CommandFactory::ListCommands(bool verbose) const
{
	for (auto const& ct : m_commandTypes)
	{
		m_log.Message("%s", ct.first.c_str());
		ct.second->LogInfo(m_log, verbose);
	}
	m_log.Message("");
	if (!verbose)
	{
		m_log.Message("Switch to verbose output for more information.");
	}
}

void CommandFactory::Log(const char* msg, const char* a)
{
	m_log.Error(msg, a);
}
