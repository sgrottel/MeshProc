#include "CmdLineArgs.h"
#include "CommandFactory.h"
#include "CommandRegistration.h"
#include "MeshProgram.h"
#include "DevPlayground.h"

#include <SimpleLog/SimpleLog.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <memory>
#include <cwctype>

using namespace meshproc;

int wmain(int argc, wchar_t **argv)
{
	sgrottel::NullLog nullLog;
	sgrottel::EchoingSimpleLog log{ nullLog };

	CmdLineArgs cmdLine;
	if (!cmdLine.Parse(log, argc, argv)) {
		return (cmdLine.m_command == CliCommand::Error)
				? 1
				: 0;
	}

	log.SetEchoDetails(cmdLine.m_verbose);

	meshproc::CommandFactory cmdFactory{ log };
	meshproc::CommandRegistration(cmdFactory, log);

	switch (cmdLine.m_command)
	{
	case CliCommand::RunScript:
	{
		meshproc::MeshProgram prog{ log };
		prog.Load(cmdLine.m_script, cmdFactory);
		prog.Execution();
	}
	break;

	case CliCommand::ValidateScript:
	{
		meshproc::MeshProgram prog{ log };
		prog.Load(cmdLine.m_script, cmdFactory);
	}
	break;

	case CliCommand::ListCommands:
		log.Message("");
		log.Message("Available commands:");
		cmdFactory.ListCommands(cmdLine.m_verbose);
		break;

	case CliCommand::DevPlayground:
		DevPlayground(log);
		break;

	default:
		log.Critical("CLI command %d not implemented", cmdLine.m_command);
		break;
	}

	return 0;
}
