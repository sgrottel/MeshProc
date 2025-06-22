#pragma once

#include "Runner.h"

namespace meshproc
{
	namespace lua
	{

		class LogFunctions : public Runner::Component<LogFunctions>
		{
		public:
			LogFunctions(Runner& owner)
				: Component<LogFunctions>{ owner }
			{ };

			bool Init();

		private:

			static int CallbackLogWrite(lua_State* lua);
			static int CallbackLogWarn(lua_State* lua);
			static int CallbackLogError(lua_State* lua);

			int WriteLogImpl(lua_State* lua, uint32_t flags);
		};

	}
}
