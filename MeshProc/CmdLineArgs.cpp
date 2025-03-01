#include "CmdLineArgs.h"

#include <SimpleLog/SimpleLog.hpp>
#include <yaclap.hpp>

namespace
{

	void LogGreeting(const sgrottel::ISimpleLog& log)
	{
		log.Message("MeshProc");
	}

	std::wstring_view Trim(std::wstring_view str)
	{
		auto b = str.begin();
		auto e = str.end();
		while (b != e && std::iswspace(*b))
		{
			b++;
		}
		while (b != e && std::iswspace(*(e - 1)))
		{
			e--;
		}
		return std::wstring_view{ b, e };
	}

}

bool meshproc::CmdLineArgs::Parse(sgrottel::ISimpleLog& log, int argc, wchar_t const* const* argv)
{
	using Parser = yaclap::Parser<wchar_t>;
	using Argument = yaclap::Argument<wchar_t>;
	using Command = yaclap::Command<wchar_t>;
	using Option = yaclap::Option<wchar_t>;
	using Switch = yaclap::Switch<wchar_t>;

	Parser parser{ L"MeshProc.exe", L"Simple Triangle Mesh Processor Command Line Utility" };

	Command cmdRun{ L"run", L"Runs a yaml script" };
	Argument argScript{ L"script", L"The input yaml script containing the processor commands to run" };
	Option optScriptArg{ L"-arg", L"key-value-pair", L"Specifies a script argument key-value-pair (separated by '=')" };

	cmdRun.Add(argScript)
		.Add(optScriptArg);

	parser.Add(cmdRun);

	Command cmdListCmds{ L"lscmd", L"List available processor commands" };
	cmdListCmds.AddAlias(L"listcommands");

	parser.Add(cmdListCmds);

	Switch swVerbose{ L"-v", L"Verbose output" };
	parser.Add(swVerbose);

	Command cmdDevPlayground{ L"devplayground", L"" };
	cmdDevPlayground.HideFromHelp();
	parser.Add(cmdDevPlayground);

	Parser::Result res = parser.Parse(argc, argv);

	if (res.HasCommand(cmdRun))
	{
		m_command = CliCommand::RunScript;

		m_script = static_cast<std::wstring_view>(res.GetArgument(argScript));

		for (auto const& a : res.GetOptionValues(optScriptArg))
		{
			auto pos = a.find(L'=');
			if (pos == std::wstring_view::npos)
			{
				m_scriptArgs.insert(std::make_pair<std::wstring_view, std::wstring_view>(Trim(a), L""));
			}
			else
			{
				std::wstring_view key{ a.substr(0, pos) };
				std::wstring_view value{ a.substr(pos + 1) };
				m_scriptArgs.insert(std::make_pair<std::wstring_view, std::wstring_view>(Trim(key), Trim(value)));
			}
		}
	}
	else if (res.HasCommand(cmdListCmds))
	{
		m_command = CliCommand::ListCommands;
	}
	else if (res.HasCommand(cmdDevPlayground))
	{
		m_command = CliCommand::DevPlayground;
	}
	else if (res.ShouldShowHelp())
	{
		m_command = CliCommand::Help;
	}

	m_verbose = res.HasSwitch(swVerbose) > 0;

	LogGreeting(log);
	parser.PrintErrorAndHelpIfNeeded(res);
	return res.IsSuccess() && !res.ShouldShowHelp();
}
