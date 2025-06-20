#include "Runner.h"

#include "AbstractCommand.h"
#include "CommandFactory.h"
#include "ParameterBinding.h"
#include "utilities/StringUtilities.h"

#include <SimpleLog/SimpleLog.hpp>

#include <lua.hpp>

#include <stdexcept>

using namespace meshproc;
using namespace meshproc::lua;

namespace
{
	constexpr intptr_t LuaThisKey = 0x16A7;

	constexpr const char* CommandObjectClass = "SGR.MeshProc.Command";

	struct CommandObject
	{
		std::shared_ptr<AbstractCommand> cmd;
	};

	static CommandObject* GetCommandObject(lua_State* lua) {
		void* ud = luaL_checkudata(lua, 1, CommandObjectClass);
		return reinterpret_cast<CommandObject*>(ud);
	}

	template<ParamType PT>
	struct LuaParamMapping;

	template<>
	struct LuaParamMapping<ParamType::UInt32>
	{
		static void PushVal(lua_State* lua, const uint32_t& v)
		{
			lua_pushinteger(lua, v);
		}
		static bool SetVal(lua_State* lua, uint32_t& tar)
		{
			if (lua_isnumber(lua, 3))
			{
				tar = static_cast<uint32_t>(lua_tonumber(lua, 3));
				return true;
			}
			if (lua_isinteger(lua, 3))
			{
				tar = static_cast<uint32_t>(lua_tointeger(lua, 3));
				return true;
			}
			return false;
		}
	};

	template<>
	struct LuaParamMapping<ParamType::Float>
	{
		static void PushVal(lua_State* lua, const float& v)
		{
			lua_pushnumber(lua, v);
		}
		static bool SetVal(lua_State* lua, float& tar)
		{
			if (lua_isnumber(lua, 3))
			{
				tar = static_cast<float>(lua_tonumber(lua, 3));
				return true;
			}
			if (lua_isinteger(lua, 3))
			{
				tar = static_cast<float>(lua_tointeger(lua, 3));
				return true;
			}
			return false;
		}
	};

	template<>
	struct LuaParamMapping<ParamType::String>
	{
		static void PushVal(lua_State* lua, const std::wstring& v)
		{
			lua_pushstring(lua, ToUtf8(v).c_str());
		}
		static bool SetVal(lua_State* lua, std::wstring& tar)
		{
			if (lua_isstring(lua, 3))
			{
				const char* str = lua_tostring(lua, 3);
				tar = FromUtf8(str);
				return true;
			}
			return false;
		}
	};

	template<ParamType PT>
	static int LuaTryPushVal(lua_State* lua, std::shared_ptr<ParameterBinding::ParamBindingBase> param, sgrottel::ISimpleLog& log)
	{
		const ParamTypeInfo_t<PT>* v = ParameterBinding::GetValueSource<PT>(param.get());
		if (v == nullptr)
		{
			log.Error("Parameter value type mismatch");
			return 0;
		}
		LuaParamMapping<PT>::PushVal(lua, *v);
		return 1;
	}

	template<ParamType PT>
	static bool LuaTrySetVal(lua_State* lua, std::shared_ptr<ParameterBinding::ParamBindingBase> param, sgrottel::ISimpleLog& log)
	{
		ParamTypeInfo_t<PT>* v = ParameterBinding::GetValueTarget<PT>(param.get());
		if (v == nullptr)
		{
			log.Error("Parameter value type mismatch");
			return false;
		}
		if (!LuaParamMapping<PT>::SetVal(lua, *v))
		{
			log.Error("Failed to set parameter value; likely type mismatch");
			return false;
		}
		return true;
	}

}

Runner::Runner(sgrottel::ISimpleLog& log, CommandFactory& factory)
	: m_log{ log }
	, m_factory{ factory }
{
	// intentionally empty
}

bool Runner::Init()
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

bool Runner::RegisterCommands()
{
	if (!AssertStateReady()) return false;

	static const struct luaL_Reg commandObjectLib_memberFunc[] = {
		{"__tostring", &Runner::CallbackCommandToString},
		{"__gc", &Runner::CallbackCommandDelete},
		{"invoke", &Runner::CallbackCommandInvoke},
		{"set", &Runner::CallbackCommandSet},
		{"get", &Runner::CallbackCommandGet},
		{nullptr, nullptr}
	};

	// The member functions for the commandObject
	luaL_newmetatable(m_state.get(), CommandObjectClass);
	lua_pushstring(m_state.get(), "__index");
	lua_pushvalue(m_state.get(), -2);	// pushes the metatable
	lua_settable(m_state.get(), -3);	// metatable.__index = metatable
	luaL_setfuncs(m_state.get(), commandObjectLib_memberFunc, 0);

	lua_newtable(m_state.get());

	lua_pushcfunction(m_state.get(), &Runner::CallbackCreateCommand);
	lua_setfield(m_state.get(), -2, "_createCommand");

	lua_setglobal(m_state.get(), "meshproc");

	for (const std::string& name : m_factory.GetAllNames())
	{
		std::string func = "meshproc." + name + " = meshproc." + name + " or {}\n"
			+ "function meshproc." + name + ".new()\n"
			+ "  return meshproc._createCommand(\"" + name + "\")\n"
			+ "end";
		if (auto dotPos = std::find(name.begin(), name.end(), '.'); dotPos != name.end())
		{
			const std::string gn = "meshproc." + name.substr(0, std::distance(name.begin(), dotPos));
			func = gn + " = " + gn + " or {}\n" + func;
		}

		if (luaL_dostring(m_state.get(), func.c_str()) != LUA_OK)
		{
			const char* errormsg = lua_tostring(m_state.get(), -1);
			m_log.Error("Failed to define lua function \"meshproc.%s\": %s", name.c_str(), errormsg);
			lua_pop(m_state.get(), 1);
		}
	}

	return true;
}

bool Runner::LoadScript(const std::filesystem::path& script)
{
	if (!AssertStateReady()) return false;

	// TODO: Support UTF16 paths!

	if (luaL_loadfile(m_state.get(), script.generic_string().c_str()))
	{
		m_log.Critical("Failed to load lua script: %s", lua_tostring(m_state.get(), -1));
		return false;
	}
	return true;
}

bool Runner::RunScript()
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

Runner* Runner::GetThis(lua_State* lua)
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
			return reinterpret_cast<Runner*>(that);
		}
	}
	catch(...)
	{
	}
	return nullptr;
}

template<typename FN, typename... ARGS>
int Runner::CallLuaImpl(FN fn, lua_State* lua, ARGS... args)
{
	auto that = GetThis(lua);
	if (that == nullptr) return 0;
	return (that->*fn)(lua, args...);
}

int Runner::CallbackLogWrite(lua_State* lua)
{
	return CallLuaImpl(&Runner::CallbackLogImpl, lua, sgrottel::ISimpleLog::FlagLevelMessage);
}

int Runner::CallbackLogWarn(lua_State* lua)
{
	return CallLuaImpl(&Runner::CallbackLogImpl, lua, sgrottel::ISimpleLog::FlagLevelWarning);
}

int Runner::CallbackLogError(lua_State* lua)
{
	return CallLuaImpl(&Runner::CallbackLogImpl, lua, sgrottel::ISimpleLog::FlagLevelError);
}

int Runner::CallbackCreateCommand(lua_State* lua)
{
	return CallLuaImpl(&Runner::CallbackCreateCommandImpl, lua);
}

int Runner::CallbackCommandDelete(lua_State* lua)
{
	CommandObject* cmdObj = GetCommandObject(lua);
	if (cmdObj != nullptr)
	{
		cmdObj->cmd.reset();
	}
	return 0;
}

int Runner::CallbackCommandToString(lua_State* lua)
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

int Runner::CallbackCommandInvoke(lua_State* lua)
{
	return CallLuaImpl(&Runner::CallbackCommandInvokeImpl, lua);
}

int Runner::CallbackCommandGet(lua_State* lua)
{
	return CallLuaImpl(&Runner::CallbackCommandGetImpl, lua);
}

int Runner::CallbackCommandSet(lua_State* lua)
{
	return CallLuaImpl(&Runner::CallbackCommandSetImpl, lua);
}

bool Runner::AssertStateReady()
{
	if (!m_state)
	{
		m_log.Critical("Lua state maschine not initialized");
		return false;
	}
	return true;
}

bool Runner::RegisterLogFunctions()
{
	if (!AssertStateReady()) return false;

	lua_newtable(m_state.get());

	// This would be for overwriting a function in an existing table
	// lua_getglobal(m_state.get(), "io"); // Get the global 'io' table

	lua_pushcfunction(m_state.get(), &Runner::CallbackLogWrite);		// Push your custom function
	lua_setfield(m_state.get(), -2, "write");			// Assign it to the 'write' field in the io table, and pops from stack
	lua_pushcfunction(m_state.get(), &Runner::CallbackLogWarn);		// Push your custom function
	lua_setfield(m_state.get(), -2, "warn");			// Assign it to the 'write' field in the io table, and pops from stack
	lua_pushcfunction(m_state.get(), &Runner::CallbackLogError);		// Push your custom function
	lua_setfield(m_state.get(), -2, "error");			// Assign it to the 'write' field in the io table, and pops from stack
	lua_setglobal(m_state.get(), "log"); // sets global chost = table, and pops table from stack

	// lua_pop(m_state.get(), 1); // Remove a loaded table from the stack

	return true;
}

int Runner::CallbackLogImpl(lua_State* lua, uint32_t flags)
{
	int nargs = lua_gettop(lua);
	for (int i = 1; i <= nargs; i++)
	{
		size_t len;
		const char* s = luaL_tolstring(lua, i, &len);
		if (s)
		{
			m_log.Write(flags & sgrottel::ISimpleLog::FlagLevelMask, L"%s", FromUtf8(s).c_str());

			lua_pop(lua, 1);	// Remove result of luaL_tolstring
		}
	}
	return 0;
}

int Runner::CallbackCreateCommandImpl(lua_State* lua)
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

	return 1; // cmdObj is on stack
}

int Runner::CallbackCommandInvokeImpl(lua_State* lua)
{
	CommandObject* cmdObj = GetCommandObject(lua);
	if (cmdObj == nullptr || cmdObj->cmd == nullptr)
	{
		return 0;
	}

	const char* name = m_factory.FindName(cmdObj->cmd.get());
	if (name == nullptr) name = "<UNKNOWN>";

	try
	{
		m_log.Detail("Invoking %s", name);
		bool rv = cmdObj->cmd->Invoke();
		if (!rv)
		{
			m_log.Warning("Invoking %s returned unsuccessful", name);
		}
		lua_pushboolean(lua, rv ? 1 : 0);
		return 1;
	}
	catch (std::exception& ex)
	{
		m_log.Error("Exception trying to invoke %s: %s", name, ex.what());
	}
	catch (...)
	{
		m_log.Error("Unknown exception trying to invoke %s", name);
	}
	return 0;
}

int Runner::CallbackCommandGetImpl(lua_State* lua)
{
	CommandObject* cmdObj = GetCommandObject(lua);
	if (cmdObj == nullptr || cmdObj->cmd == nullptr) return 0;

	int nargs = lua_gettop(lua);
	if (nargs != 2)
	{
		m_log.Error("Field name argument missing");
		return 0;
	}
	if (!lua_isstring(lua, 2))
	{
		m_log.Error("Field name argument of wrong type");
		return 0;
	}

	size_t len;
	std::string name = luaL_tolstring(lua, 2, &len); // copy type string

	std::shared_ptr<ParameterBinding::ParamBindingBase> param = cmdObj->cmd->GetParam(name);
	if (!param)
	{
		m_log.Error("Field name %s not found", name.c_str());
		return 0;
	}

	switch (param->m_type)
	{
	case ParamType::UInt32:
		return LuaTryPushVal<ParamType::UInt32>(lua, param, m_log);
	case ParamType::Float:
		return LuaTryPushVal<ParamType::Float>(lua, param, m_log);
	case ParamType::String:
		return LuaTryPushVal<ParamType::String>(lua, param, m_log);
	//	Vec3,
	//	Mat4,
	//	Mesh,
	//	MultiMesh,
	//	Scene,
	//	VertexSelection, // e.g. also edges/loops
	//	MultiVertexSelection,
	default:
		m_log.Error("Getting field %s value of type %s is not supported", name.c_str(), GetParamTypeName(param->m_type));
		return 0;
	}

	return 1;
}

int Runner::CallbackCommandSetImpl(lua_State* lua)
{
	CommandObject* cmdObj = GetCommandObject(lua);
	if (cmdObj == nullptr || cmdObj->cmd == nullptr) return 0;

	int nargs = lua_gettop(lua);
	if (nargs != 3)
	{
		m_log.Error("Field name argument missing");
		return 0;
	}
	if (!lua_isstring(lua, 3))
	{
		m_log.Error("Field name argument of wrong type");
		return 0;
	}

	size_t len;
	std::string name = luaL_tolstring(lua, 2, &len); // copy type string

	std::shared_ptr<ParameterBinding::ParamBindingBase> param = cmdObj->cmd->GetParam(name);
	if (!param)
	{
		m_log.Error("Field name %s not found", name.c_str());
		return 0;
	}
	if (param->m_mode == ParamMode::Out)
	{
		m_log.Error("Field name %s is read only", name.c_str());
		return 0;
	}

	switch (param->m_type)
	{
	case ParamType::UInt32:
		LuaTrySetVal<ParamType::UInt32>(lua, param, m_log);
		break;
	case ParamType::Float:
		LuaTrySetVal<ParamType::Float>(lua, param, m_log);
		break;
	case ParamType::String:
		LuaTrySetVal<ParamType::String>(lua, param, m_log);
		break;
		//	Vec3,
		//	Mat4,
		//	Mesh,
		//	MultiMesh,
		//	Scene,
		//	VertexSelection, // e.g. also edges/loops
		//	MultiVertexSelection,
	default:
		m_log.Error("Setting field %s value of type %s is not supported", name.c_str(), GetParamTypeName(param->m_type));
		return 0;
	}

	return 0;
}
