#pragma once

#include "Runner.h"

namespace meshproc
{
	class CommandFactory;

	namespace lua
	{

		class CommandCreator : public Runner::Component<CommandCreator>
		{
		public:
			CommandCreator(Runner& owner, const CommandFactory& factory)
				: Component<CommandCreator>{ owner }
				, m_factory{ factory }
			{};

			bool RegisterCommands();

		private:

			static int CallbackCreateCommand(lua_State* lua);

			int CreateCommandImpl(lua_State* lua);
			
			const CommandFactory& m_factory;
		};

	}
}
