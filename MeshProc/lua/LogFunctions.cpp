#include "LogFunctions.h"

#include "utilities/StringUtilities.h"

#include <SimpleLog/SimpleLog.hpp>

#include <lua.hpp>

using namespace meshproc;
using namespace meshproc::lua;

bool LogFunctions::Init()
{
	if (!AssertStateReady()) return false;

	lua_newtable(lua());

	// This would be for overwriting a function in an existing table
	// lua_getglobal(m_state.get(), "io"); // Get the global 'io' table

	lua_pushcfunction(lua(), &LogFunctions::CallbackLogWrite);		// Push your custom function
	lua_setfield(lua(), -2, "write");			// Assign it to the 'write' field in the io table, and pops from stack
	lua_pushcfunction(lua(), &LogFunctions::CallbackLogDetail);		// Push your custom function
	lua_setfield(lua(), -2, "detail");			// Assign it to the 'write' field in the io table, and pops from stack
	lua_pushcfunction(lua(), &LogFunctions::CallbackLogWarn);		// Push your custom function
	lua_setfield(lua(), -2, "warn");			// Assign it to the 'write' field in the io table, and pops from stack
	lua_pushcfunction(lua(), &LogFunctions::CallbackLogError);		// Push your custom function
	lua_setfield(lua(), -2, "error");			// Assign it to the 'write' field in the io table, and pops from stack
	lua_setglobal(lua(), "log"); // sets global chost = table, and pops table from stack

	// lua_pop(m_state.get(), 1); // Remove a loaded table from the stack

	return true;

}

int LogFunctions::CallbackLogWrite(lua_State* lua)
{
	return CallLuaImpl(&LogFunctions::WriteLogImpl, lua, sgrottel::ISimpleLog::FlagLevelMessage);
}

int LogFunctions::CallbackLogDetail(lua_State* lua)
{
	return CallLuaImpl(&LogFunctions::WriteLogImpl, lua, sgrottel::ISimpleLog::FlagLevelDetail);
}

int LogFunctions::CallbackLogWarn(lua_State* lua)
{
	return CallLuaImpl(&LogFunctions::WriteLogImpl, lua, sgrottel::ISimpleLog::FlagLevelWarning);
}

int LogFunctions::CallbackLogError(lua_State* lua)
{
	return CallLuaImpl(&LogFunctions::WriteLogImpl, lua, sgrottel::ISimpleLog::FlagLevelError);
}

int LogFunctions::WriteLogImpl(lua_State* lua, uint32_t flags)
{
	int nargs = lua_gettop(lua);
	for (int i = 1; i <= nargs; i++)
	{
		size_t len;
		const char* s = luaL_tolstring(lua, i, &len);
		if (s)
		{
			Log().Write(flags & sgrottel::ISimpleLog::FlagLevelMask, L"%s", FromUtf8(s).c_str());

			lua_pop(lua, 1);	// Remove result of luaL_tolstring
		}
	}
	return 0;
}
