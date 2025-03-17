#pragma once

#include <filesystem>
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

		inline void Clear();
		void Load(std::filesystem::path script, CommandFactory const& factory);

		void Execution() const;

	private:

		struct Instruction
		{
			std::string name;
			std::shared_ptr<AbstractCommand> cmd;
			std::unordered_map<std::string, std::string> setVarParam;
			std::unordered_map<std::string, std::string> setConstParam;
			std::unordered_map<std::string, std::string> getVarParam;
		};

		const sgrottel::ISimpleLog& m_log;
		std::vector<Instruction> m_program;
	};

	void MeshProgram::Clear()
	{
		m_program.clear();
	}

}
