#include "Runner.h"

#include "CommandCreator.h"
#include "CommandType.h"
#include "LogFunctions.h"
#include "MeshType.h"
#include "SceneType.h"

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
}

class Runner::Components {
public:
	Components(Runner& owner, const CommandFactory& factory)
		: m_commandCreator{ owner, factory }
		, m_commandType{ owner }
		, m_logFunctions{ owner }
		, m_meshType{ owner }
		, m_sceneType{ owner }
	{ }

	bool Init();

	CommandCreator m_commandCreator;
	CommandType m_commandType;
	LogFunctions m_logFunctions;
	MeshType m_meshType;
	SceneType m_sceneType;
};

bool Runner::Components::Init()
{
	if (!m_logFunctions.Init()) return false;
	if (!m_commandType.Init()) return false;
	if (!m_meshType.Init()) return false;
	if (!m_sceneType.Init()) return false;
	return true;
}

#define IMPL_RUNNER_GET_COMPONENT(TYPE, NAME)	\
	template<>									\
	TYPE* Runner::GetComponent<TYPE>() const	\
	{											\
		if (!m_components) return nullptr;		\
		return &m_components->NAME;				\
	}

IMPL_RUNNER_GET_COMPONENT(CommandCreator, m_commandCreator)
IMPL_RUNNER_GET_COMPONENT(CommandType, m_commandType)
IMPL_RUNNER_GET_COMPONENT(LogFunctions, m_logFunctions)
IMPL_RUNNER_GET_COMPONENT(MeshType, m_meshType)
IMPL_RUNNER_GET_COMPONENT(SceneType, m_sceneType)

Runner::Runner(sgrottel::ISimpleLog& log, CommandFactory& factory)
	: m_log{ log }
	, m_components{}
{
	m_components = std::make_shared<Components>(*this, factory);
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

	if (!m_components) return false;
	if (!m_components->Init()) return false;

	return true;
}

bool Runner::RegisterCommands()
{
	if (!AssertStateReady()) return false;
	if (!m_components) return false;
	return m_components->m_commandCreator.RegisterCommands();
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

bool Runner::AssertStateReady()
{
	if (!m_state)
	{
		m_log.Critical("Lua state maschine not initialized");
		return false;
	}
	return true;
}
