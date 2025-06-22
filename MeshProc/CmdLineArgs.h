#pragma once

#include <string_view>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{

	enum class CliCommand
	{
		Error,
		RunScript,
		ListCommands,
		Help
	};

	class CmdLineArgs
	{
	public:
		CliCommand m_command{ CliCommand::Error };
		bool m_verbose{ false };
		std::filesystem::path m_script{};
		std::unordered_map<std::wstring_view, std::wstring_view> m_scriptArgs{};

		bool Parse(sgrottel::ISimpleLog& log, int argc, wchar_t const* const* argv);
	};

}
