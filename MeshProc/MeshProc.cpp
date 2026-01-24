#include "CmdLineArgs.h"
#include "commands/CommandFactory.h"
#include "commands/CommandRegistration.h"
#include "lua/Runner.h"

#include <SimpleLog/SimpleLog.hpp>

int wmain(int argc, wchar_t **argv)
{
	using meshproc::CmdLineArgs;
	using meshproc::CliCommand;

	sgrottel::NullLog nullLog;
	sgrottel::EchoingSimpleLog log{ nullLog };

	CmdLineArgs cmdLine;
	if (!cmdLine.Parse(log, argc, argv)) {
		return (cmdLine.m_command == CliCommand::Error)
				? 1
				: 0;
	}

	log.SetEchoDetails(cmdLine.m_verbose);

	meshproc::commands::CommandFactory cmdFactory{ log };
	meshproc::commands::CommandRegistration(cmdFactory, log);

	switch (cmdLine.m_command)
	{
	case CliCommand::RunScript:
	{
		meshproc::lua::Runner lua{ log, cmdFactory };
		if (!lua.Init()) break;
		if (!lua.RegisterCommands()) break;

		log.Detail(L"Loading Lua: %s", cmdLine.m_script.wstring().c_str());
		if (!lua.LoadScript(cmdLine.m_script)) break;
		if (!lua.SetArgs(cmdLine.m_scriptArgs)) break;
		if (!lua.RunScript()) break;

		log.Message("done.");
	}
	break;

	case CliCommand::ListCommands:
		log.Message("");
		log.Message("Available commands:");
		cmdFactory.ListCommands(cmdLine.m_verbose);
		break;

	default:
		log.Critical("CLI command %d not implemented", cmdLine.m_command);
		break;
	}

	return 0;
}
