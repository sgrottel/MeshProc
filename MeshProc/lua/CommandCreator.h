#pragma once

#include "Runner.h"

namespace meshproc
{
	namespace commands
	{
		class CommandFactory;
	}

	namespace lua
	{

		class CommandCreator : public Runner::Component<CommandCreator>
		{
		public:
			CommandCreator(Runner& owner, const commands::CommandFactory& factory)
				: Component<CommandCreator>{ owner }
				, m_factory{ factory }
			{};

			bool RegisterCommands();

		private:

			static int CallbackCreateCommand(lua_State* lua);

			int CreateCommandImpl(lua_State* lua);
			
			const commands::CommandFactory& m_factory;
		};

	}
}
