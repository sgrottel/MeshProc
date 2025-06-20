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
		bool Register(const char* name);

		void ListCommands(bool verbose) const;

		std::shared_ptr<class AbstractCommand> Instantiate(const std::string& name, const sgrottel::ISimpleLog& log) const;

		const char* FindName(const class AbstractCommand* cmd) const;

		std::vector<std::string> GetAllNames() const;

	private:
		class CommandTemplateBase
		{
		public:
			virtual ~CommandTemplateBase() = default;

			virtual std::shared_ptr<class AbstractCommand> Instantiate(const sgrottel::ISimpleLog& log) = 0;

			virtual bool IsTypeOf(const class AbstractCommand* cmd) const = 0;

		protected:
			CommandTemplateBase() = default;
		};

		template<typename TCMD>
		class CommandTemplate : public CommandTemplateBase
		{
		public:
			CommandTemplate() = default;

			std::shared_ptr<class AbstractCommand> Instantiate(const sgrottel::ISimpleLog& log) override
			{
				return std::make_shared<TCMD>(log);
			}

			bool IsTypeOf(const class AbstractCommand* cmd) const override
			{
				return dynamic_cast<const TCMD*>(cmd) != nullptr;
			}
		};

		void Log(const char* msg, const char* a);

		const sgrottel::ISimpleLog& m_log;
		std::unordered_map<std::string, std::unique_ptr<CommandTemplateBase>> m_commandTemplates;
	};

	template<typename TCMD>
	bool CommandFactory::Register(const char* name)
	{
		std::string n{ name };
		if (m_commandTemplates.find(name) != m_commandTemplates.end())
		{
			Log("Cannot register command `%s`, name already registered", name);
			throw std::logic_error("Cannot register command, name already registered");
			return false;
		}
		m_commandTemplates.insert({ n, std::make_unique<CommandTemplate<TCMD>>() });
		return true;
	}

}
