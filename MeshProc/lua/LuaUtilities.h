#pragma once

// forward declaration
typedef struct lua_State lua_State;

namespace meshproc
{
	namespace lua
	{
		void DumpLuaStack(lua_State* lua);
	}
}
