#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{
	class AbstractCommand;

	class CommandFactory
	{
	public:
		CommandFactory(const sgrottel::ISimpleLog& log);

		template<typename T>
		bool Register(const char* name)
		{
			std::string n{ name };
			if (m_commandTypes.find(name) != m_commandTypes.end())
			{
				Log("Cannot register command `%s`, name already registered", name);
				throw std::logic_error("Cannot register command, name already registered");
				return false;
			}
			m_commandTypes.insert({ n, std::make_shared<T>(MyNullLog()) });
			return true;
		}

		void ListCommands(bool verbose) const;

	private:
		static sgrottel::ISimpleLog& MyNullLog();

		void Log(const char* msg, const char* a);

		const sgrottel::ISimpleLog& m_log;
		std::unordered_map<std::string, std::shared_ptr<AbstractCommand>> m_commandTypes;
	};

}
