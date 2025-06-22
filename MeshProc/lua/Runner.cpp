#include "Runner.h"

#include "CommandCreator.h"
#include "CommandType.h"
#include "LogFunctions.h"
#include "LuaResources.h"
#include "MeshType.h"
#include "MultiMeshType.h"
#include "MultiVertexSelectionType.h"
#include "SceneType.h"
#include "VertexSelectionType.h"

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
		, m_multiMeshType{ owner }
		, m_multiVertexSelectionType{ owner }
		, m_sceneType{ owner }
		, m_vertexSelectionType{ owner }
	{ }

	bool Init();

	CommandCreator m_commandCreator;
	CommandType m_commandType;
	LogFunctions m_logFunctions;
	MeshType m_meshType;
	MultiMeshType m_multiMeshType;
	MultiVertexSelectionType m_multiVertexSelectionType;
	SceneType m_sceneType;
	VertexSelectionType m_vertexSelectionType;
};

bool Runner::Components::Init()
{
	if (!m_logFunctions.Init()) return false;
	if (!m_commandType.Init()) return false;
	if (!m_meshType.Init()) return false;
	if (!m_multiMeshType.Init()) return false;
	if (!m_multiVertexSelectionType.Init()) return false;
	if (!m_sceneType.Init()) return false;
	if (!m_vertexSelectionType.Init()) return false;
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
IMPL_RUNNER_GET_COMPONENT(MultiMeshType, m_multiMeshType)
IMPL_RUNNER_GET_COMPONENT(MultiVertexSelectionType, m_multiVertexSelectionType)
IMPL_RUNNER_GET_COMPONENT(SceneType, m_sceneType)
IMPL_RUNNER_GET_COMPONENT(VertexSelectionType, m_vertexSelectionType)

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

	// load default libs
	luaL_openlibs(m_state.get());

	// register lib-loader callback function for embedded libs
	lua_getglobal(m_state.get(), "package");
	lua_getfield(m_state.get(), -1, "searchers");
	lua_pushcfunction(m_state.get(), &Runner::LuaLibLoader);
	lua_rawseti(m_state.get(), -2, luaL_len(m_state.get(), -2) + 1);
	lua_pop(m_state.get(), 2);

	// initialize runner main global
	lua_newtable(m_state.get());
	lua_setglobal(m_state.get(), "meshproc");

	// initialize components
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

bool Runner::SetArgs(const std::unordered_map<std::wstring_view, std::wstring_view>& args)
{
	lua_newtable(m_state.get());

	for (const auto& arg : args)
	{
		lua_pushstring(m_state.get(), ToUtf8(arg.second).c_str());
		lua_setfield(m_state.get(), -2, ToUtf8(arg.first).c_str());
	}

	lua_setglobal(m_state.get(), "args");
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

int Runner::LuaLibLoader(lua_State* lua)
{
	const char* module_name = luaL_checkstring(lua, 1);

	if (strcmp(module_name, "xyz_math") == 0)
	{
		HRSRC res = FindResourceW(nullptr, MAKEINTRESOURCE(LUA_LIB_XYZ_MATH), RT_RCDATA);
		if (res == nullptr)
		{
			return 0;
		}

		HGLOBAL resHandle = LoadResource(nullptr, res);
		if (resHandle == nullptr)
		{
			return 0;
		}

		void* data = LockResource(resHandle);
		if (data == nullptr)
		{
			return 0;
		}

		DWORD size = SizeofResource(nullptr, res);
		if (size == 0)
		{
			return 0;
		}

		std::string libData{ static_cast<const char*>(data), size };

		const std::string wrong = "XMat4.new(unpack(c))";
		const std::string fix = "XMat4.new(table.unpack(c))";
		size_t start_pos = 0;
		while ((start_pos = libData.find(wrong, start_pos)) != std::string::npos) {
			libData.replace(start_pos, wrong.length(), fix);
			start_pos += fix.length();
		}

		libData += R"(
debug.getregistry()["XVec2_mt"] = getmetatable(XVec2(0, 0))
debug.getregistry()["XVec3_mt"] = getmetatable(XVec3(0, 0, 0))
debug.getregistry()["XVec4_mt"] = getmetatable(XVec4(0, 0, 0, 1))
debug.getregistry()["XMat3_mt"] = getmetatable(XMat3.new())
debug.getregistry()["XMat4_mt"] = getmetatable(XMat4.new())
)";
		if (luaL_loadbuffer(lua, libData.c_str(), libData.size(), "lib") != LUA_OK)
		{
			// signal error instead of returning lib init function
			lua_pushstring(lua, lua_tostring(lua, -1));
		}

		return 1;
	}

	return 0; // Let Lua continue trying other searchers
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
