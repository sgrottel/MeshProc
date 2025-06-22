#pragma once

#include "AbstractType.h"

namespace meshproc
{
	class AbstractCommand;

	namespace lua
	{

		class CommandType : public AbstractType<AbstractCommand, CommandType>
		{
		public:
			static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Command";

			CommandType(Runner& owner)
				: AbstractType<AbstractCommand, CommandType>{ owner }
			{ };

			bool Init();
		private:

			static int CallbackCommandToString(lua_State* lua);
			static int CallbackCommandInvoke(lua_State* lua);
			static int CallbackCommandGet(lua_State* lua);
			static int CallbackCommandSet(lua_State* lua);

			int InvokeImpl(lua_State* lua);
			int GetImpl(lua_State* lua);
			int SetImpl(lua_State* lua);

		};

	}
}
