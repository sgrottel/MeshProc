#include "CmdLineArgs.h"

#include <SimpleLog/SimpleLog.hpp>
#include <yaclap.hpp>

namespace
{

	void LogGreeting(sgrottel::ISimpleLog& log)
	{
		log.Message("MeshProc");
	}

}

bool CmdLineArgs::Parse(sgrottel::ISimpleLog& log, int argc, wchar_t const* const* argv)
{
	using Parser = yaclap::Parser<wchar_t>;
	using Option = yaclap::Option<wchar_t>;

	Parser parser{ L"MeshProc.exe", L"Simple Triangle Mesh Processor Command Line Utility" };

	Option inputFile{ L"-input", L"file", L"The file to read as input" };
	parser.Add(inputFile);

	Parser::Result res = parser.Parse(argc, argv);

	if (res.GetOptionCount(inputFile) > 0)
	{
		input.assign<std::wstring_view>(res.GetOptionValue(inputFile));
	}

	// TODO: Implement

	LogGreeting(log);
	parser.PrintErrorAndHelpIfNeeded(res);
	return res.IsSuccess() && !res.ShouldShowHelp();
}
