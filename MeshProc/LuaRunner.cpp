#include "LuaRunner.h"

#include "AbstractCommand.h"
#include "CommandFactory.h"

#include <SimpleLog/SimpleLog.hpp>

#include <lua.hpp>

#include <stdexcept>

using namespace meshproc;

namespace
{
	constexpr intptr_t LuaThisKey = 0x16A7;

	constexpr const char* CommandObjectClass = "SGR.MeshProc.Command";

	struct CommandObject
	{
		std::shared_ptr<AbstractCommand> cmd;
		int value;
	};

	static CommandObject* GetCommandObject(lua_State* lua) {
		void* ud = luaL_checkudata(lua, 1, CommandObjectClass);
		return reinterpret_cast<CommandObject*>(ud);
	}

}

LuaRunner::LuaRunner(sgrottel::ISimpleLog& log, CommandFactory& factory)
	: m_log{ log }
	, m_factory{ factory }
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


static int setMyObjectValue(lua_State* L)
{
	CommandObject* mo = GetCommandObject(L);
	int value = static_cast<int>(luaL_checkinteger(L, 2));
	mo->value = value;
	return 0;
}

static int getMyObjectValue(lua_State* L)
{
	CommandObject* mo = GetCommandObject(L);
	lua_pushinteger(L, mo->value);
	return 1;
}


bool LuaRunner::RegisterCommands()
{
	if (!AssertStateReady()) return false;

	static const struct luaL_Reg commandObjectLib_memberFunc[] = {
		{"__tostring", &LuaRunner::CallbackCommandToString},
		{"__gc", &LuaRunner::CallbackCommandDelete},
		{"set", setMyObjectValue},
		{"get", getMyObjectValue},
		{NULL, NULL}
	};

	// The member functions for the commandObject
	luaL_newmetatable(m_state.get(), CommandObjectClass);
	lua_pushstring(m_state.get(), "__index");
	lua_pushvalue(m_state.get(), -2);	// pushes the metatable
	lua_settable(m_state.get(), -3);	// metatable.__index = metatable
	luaL_setfuncs(m_state.get(), commandObjectLib_memberFunc, 0);

	lua_newtable(m_state.get());

	lua_pushcfunction(m_state.get(), &LuaRunner::CallbackCreateCommand);
	lua_setfield(m_state.get(), -2, "_createCommand");

	lua_setglobal(m_state.get(), "meshproc");

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

int LuaRunner::CallbackCreateCommand(lua_State* lua)
{
	auto that = GetThis(lua);
	if (that == nullptr) return 0;
	return that->CallbackCreateCommandImpl(lua);
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

int LuaRunner::CallbackCreateCommandImpl(lua_State* lua)
{
	int nargs = lua_gettop(lua);
	if (nargs != 1)
	{
		m_log.Error("Type argument for CreateCommand function missing");
		return 0;
	}
	if (!lua_isstring(lua, 1))
	{
		m_log.Error("Type argument for CreateCommand function of wrong type");
		return 0;
	}

	size_t len;
	std::string typeStr = luaL_tolstring(lua, 1, &len); // copy type string
	lua_pop(lua, 1);	// Remove result of luaL_tolstring

	m_log.Detail("CreateCommand(%s)", typeStr.c_str());

	std::shared_ptr<AbstractCommand> cmd = m_factory.Instantiate(typeStr, m_log);
	if (!cmd)
	{
		m_log.Error("Failed to create command '%s'", typeStr.c_str());
		return 0;
	}

	CommandObject* cmdObj = (CommandObject*)lua_newuserdata(lua, sizeof(CommandObject)); // put on stack
	memset(cmdObj, 0, sizeof(CommandObject));

	luaL_getmetatable(lua, CommandObjectClass);
	lua_setmetatable(lua, -2);
	cmdObj->cmd = cmd;
	cmdObj->value = 42;

	return 1; // cmdObj is on stack
}

int LuaRunner::CallbackCommandDelete(lua_State* lua)
{
	CommandObject* cmdObj = GetCommandObject(lua);
	if (cmdObj != nullptr)
	{
		cmdObj->cmd.reset();
	}
	return 0;
}

int LuaRunner::CallbackCommandToString(lua_State* lua)
{
	CommandObject* cmdObj = GetCommandObject(lua);
	if (cmdObj == nullptr || cmdObj->cmd == nullptr)
	{
		lua_pushstring(lua, "nil");
		return 1;
	}

	auto that = GetThis(lua);
	if (that == nullptr)
	{
		lua_pushstring(lua, "nil");
		return 1;
	}

	const char* name = that->m_factory.FindName(cmdObj->cmd.get());
	if (name == nullptr)
	{
		lua_pushstring(lua, "nil");
		return 1;
	}

	lua_pushstring(lua, name);
	return 1;
}
