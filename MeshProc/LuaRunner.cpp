#include "LuaRunner.h"

#include <SimpleLog/SimpleLog.hpp>

#include <lua.hpp>

#include <stdexcept>

using namespace meshproc;

namespace
{
	constexpr intptr_t LuaThisKey = 0x16A7;
}

LuaRunner::LuaRunner(sgrottel::ISimpleLog& log)
	: m_log{ log }
{
	// intentionally empty
}

bool LuaRunner::Init()
{
	if (m_state)
	{
		m_log.Critical("Lua state maschine is already initialized");
		return false;
	}

	m_state = std::shared_ptr<lua_State>(luaL_newstate(), &luaL_openlibs);

	// store the 'this' back reference in the lua state object
	lua_pushlightuserdata(m_state.get(), reinterpret_cast<void*>(LuaThisKey));
	lua_pushlightuserdata(m_state.get(), reinterpret_cast<void*>(this));
	lua_settable(m_state.get(), LUA_REGISTRYINDEX);

	luaL_openlibs(m_state.get());

	if (!RegisterLogFunctions()) return false;

	return true;
}

bool LuaRunner::LoadScript(const std::filesystem::path& script)
{
	if (!AssertStateReady()) return false;
	if (luaL_loadfile(m_state.get(), script.generic_string().c_str()))
	{
		m_log.Critical("Failed to load lua script: %s", lua_tostring(m_state.get(), -1));
		return false;
	}
	return true;
}

bool LuaRunner::RunScript()
{
	if (!AssertStateReady()) return false;
	try
	{
		if (lua_pcall(m_state.get(),
			0,	// no arguments to function
			0,	// no expected return values
			0	// default error function
		))
		{
			m_log.Critical("Failed to run lua script: %s", lua_tostring(m_state.get(), -1));
			return false;
		}
	}
	catch (const std::exception& ex)
	{
		m_log.Critical("Exception while running lua script: %s", ex.what());
		return false;
	}
	catch (...)
	{
		m_log.Critical("Unknown exception while running lua script");
		return false;
	}
	return true;
}

LuaRunner* LuaRunner::GetThis(lua_State* lua)
{
	if (lua == nullptr) return nullptr;
	try
	{
		lua_pushlightuserdata(lua, reinterpret_cast<void*>(LuaThisKey));
		lua_gettable(lua, LUA_REGISTRYINDEX);	// load table onto stack
		void* that = lua_touserdata(lua, -1);
		lua_pop(lua, 1);	// pop table from stack
		if (that != nullptr)
		{
			return reinterpret_cast<LuaRunner*>(that);
		}
	}
	catch(...)
	{
	}
	return nullptr;
}

int LuaRunner::CallbackLogWrite(lua_State* lua)
{
	auto that = GetThis(lua);
	if (that != nullptr) that->CallbackLogImpl(lua, sgrottel::ISimpleLog::FlagLevelMessage);
	return 0;
}

int LuaRunner::CallbackLogWarn(lua_State* lua)
{
	auto that = GetThis(lua);
	if (that != nullptr) that->CallbackLogImpl(lua, sgrottel::ISimpleLog::FlagLevelWarning);
	return 0;
}

int LuaRunner::CallbackLogError(lua_State* lua)
{
	auto that = GetThis(lua);
	if (that != nullptr) that->CallbackLogImpl(lua, sgrottel::ISimpleLog::FlagLevelError);
	return 0;
}

bool LuaRunner::AssertStateReady()
{
	if (!m_state)
	{
		m_log.Critical("Lua state maschine not initialized");
		return false;
	}
	return true;
}

bool LuaRunner::RegisterLogFunctions()
{
	if (!AssertStateReady()) return false;

	lua_newtable(m_state.get());

	// This would be for overwriting a function in an existing table
	// lua_getglobal(m_state.get(), "io"); // Get the global 'io' table

	lua_pushcfunction(m_state.get(), &LuaRunner::CallbackLogWrite);		// Push your custom function
	lua_setfield(m_state.get(), -2, "write");			// Assign it to the 'write' field in the io table, and pops from stack
	lua_pushcfunction(m_state.get(), &LuaRunner::CallbackLogWarn);		// Push your custom function
	lua_setfield(m_state.get(), -2, "warn");			// Assign it to the 'write' field in the io table, and pops from stack
	lua_pushcfunction(m_state.get(), &LuaRunner::CallbackLogError);		// Push your custom function
	lua_setfield(m_state.get(), -2, "error");			// Assign it to the 'write' field in the io table, and pops from stack
	lua_setglobal(m_state.get(), "log"); // sets global chost = table, and pops table from stack

	// lua_pop(m_state.get(), 1); // Remove a loaded table from the stack

}

void LuaRunner::CallbackLogImpl(lua_State* lua, uint32_t flags)
{
	int nargs = lua_gettop(lua);
	for (int i = 1; i <= nargs; i++)
	{
		size_t len;
		const char* s = luaL_tolstring(lua, i, &len);
		if (s)
		{
			m_log.Write(flags & sgrottel::ISimpleLog::FlagLevelMask, "%s", s);

			lua_pop(lua, 1);	// Remove result of luaL_tolstring
		}
	}
}
