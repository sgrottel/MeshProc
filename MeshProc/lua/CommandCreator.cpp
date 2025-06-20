#include "CommandCreator.h"

#include "CommandType.h"

#include "CommandFactory.h"

#include <SimpleLog/SimpleLog.hpp>

#include <lua.hpp>

using namespace meshproc;
using namespace meshproc::lua;

bool CommandCreator::RegisterCommands()
{
	if (!AssertStateReady()) return false;

	lua_newtable(lua());

	lua_pushcfunction(lua(), &CommandCreator::CallbackCreateCommand);
	lua_setfield(lua(), -2, "_createCommand");

	lua_setglobal(lua(), "meshproc");

	for (const std::string& name : m_factory.GetAllNames())
	{
		std::string func = "meshproc." + name + " = meshproc." + name + " or {}\n"
			+ "function meshproc." + name + ".new()\n"
			+ "  return meshproc._createCommand(\"" + name + "\")\n"
			+ "end";
		if (auto dotPos = std::find(name.begin(), name.end(), '.'); dotPos != name.end())
		{
			do
			{
				const std::string gn = "meshproc." + name.substr(0, std::distance(name.begin(), dotPos));
				func = gn + " = " + gn + " or {}\n" + func;
				dotPos = std::find(dotPos + 1, name.end(), '.');
			}
			while (dotPos != name.end());
		}

		if (luaL_dostring(lua(), func.c_str()) != LUA_OK)
		{
			const char* errormsg = lua_tostring(lua(), -1);
			Log().Error("Failed to define lua function \"meshproc.%s\": %s", name.c_str(), errormsg);
			lua_pop(lua(), 1);
		}
	}

	return true;
}

int CommandCreator::CallbackCreateCommand(lua_State* lua)
{
	return CallLuaImpl(&CommandCreator::CreateCommandImpl, lua);
}

int CommandCreator::CreateCommandImpl(lua_State* lua)
{
	CommandType* cmdTypeComp = GetComponent<CommandType>();
	if (cmdTypeComp == nullptr)
	{
		Log().Critical("Cannot access command type component");
		return 0;
	}
	int nargs = lua_gettop(lua);
	if (nargs != 1)
	{
		Log().Error("Type argument for CreateCommand function missing");
		return 0;
	}
	if (!lua_isstring(lua, 1))
	{
		Log().Error("Type argument for CreateCommand function of wrong type");
		return 0;
	}

	size_t len;
	std::string typeStr = luaL_tolstring(lua, 1, &len); // copy type string
	lua_pop(lua, 1);	// Remove result of luaL_tolstring

	Log().Detail("CreateCommand(%s)", typeStr.c_str());

	std::shared_ptr<AbstractCommand> cmd = m_factory.Instantiate(typeStr, Log());
	if (!cmd)
	{
		Log().Error("Failed to create command '%s'", typeStr.c_str());
		return 0;
	}

	return CommandType::LuaPush(lua, cmd); // cmdObj is on stack
}
