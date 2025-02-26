#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{
	class AbstractCommand;
	class CommandFactory;

	class MeshProgram
	{
	public:
		MeshProgram(const sgrottel::ISimpleLog& log);

		void Load(CommandFactory const& factory);

		void Execution() const;

	private:

		struct Instruction
		{
			std::shared_ptr<AbstractCommand> cmd;
			std::unordered_map<std::string, std::string> setVarParam;
			std::unordered_map<std::string, std::string> setConstParam;
			std::unordered_map<std::string, std::string> getVarParam;
		};

		const sgrottel::ISimpleLog& m_log;
		std::vector<Instruction> m_program;
	};

}
