#include "Runner.h"

#include "CommandCreator.h"
#include "LuaResources.h"

//#include "ListOfFloatType.h"
#include "LogFunctions.h"
//#include "MultiMeshType.h"
//#include "Shape2DType.h"
#include "VersionCheck.h"

#include "types/CommandType.h"
#include "types/HalfSpaceType.h"
#include "types/IndexListListType.h"
#include "types/IndexListType.h"
#include "types/SceneType.h"
#include "types/MeshType.h"
#include "types/GlmVec3ListType.h"

#define IMPL_COMPONENTS(FUNC) \
/*	FUNC(ListOfFloatType)*/ \
	FUNC(LogFunctions) \
/*	FUNC(MultiMeshType) \
	FUNC(Shape2DType) */ \
	FUNC(VersionCheck) \
	FUNC(types, CommandType) \
	FUNC(types, HalfSpaceType) \
	FUNC(types, GlmVec3ListType) \
	FUNC(types, IndexListType) \
	FUNC(types, IndexListListType) \
	FUNC(types, MeshType) \
	FUNC(types, SceneType)

#include "commands/AbstractCommand.h"
#include "commands/CommandFactory.h"
#include "commands/ParameterBinding.h"
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

// namespaced class
#define NSC1(a) a
#define NSC2(a,b) a##::##b
#define NSC3(a,b,c) a##::##b##::##c
// underscore member name
#define UMN1(a) m_##a
#define UMN2(a,b) m_##a##_##b
#define UMN3(a,b,c) m_##a##_##b##_##c
// selection helper macros
#define SELECT(_1,_2,_3,NAME,...) NAME
#define EVALNAME(a) a
#define TYPENSC(...) EVALNAME(SELECT(__VA_ARGS__, NSC3, NSC2, NSC1)(__VA_ARGS__))
#define MEMBERNAME(...) EVALNAME(SELECT(__VA_ARGS__, UMN3, UMN2, UMN1)(__VA_ARGS__))

#define RUNNER_CTORCALL(...) , MEMBERNAME(__VA_ARGS__){ owner }
#define RUNNER_MEMBERDEFINITION(...) TYPENSC(__VA_ARGS__) MEMBERNAME(__VA_ARGS__);
#define RUNNER_INITIMPL(...) if(!MEMBERNAME(__VA_ARGS__).Init()) { return false; }

class Runner::Components {
public:
	Components(Runner& owner, const commands::CommandFactory& factory)
		: m_CommandCreator { owner, factory }
		IMPL_COMPONENTS(RUNNER_CTORCALL)
	{ }

	bool Init();

	CommandCreator m_CommandCreator;
	IMPL_COMPONENTS(RUNNER_MEMBERDEFINITION)
};

bool Runner::Components::Init()
{
	// if (!m_CommandType.Init()) return false;
	IMPL_COMPONENTS(RUNNER_INITIMPL)
	return true;
}

#define IMPL_RUNNER_GET_COMPONENT(...)	\
	template<>	\
	TYPENSC(__VA_ARGS__)* Runner::GetComponent<TYPENSC(__VA_ARGS__)>() const	\
	{	\
		if (!m_components) return nullptr;	\
		return &m_components->MEMBERNAME(__VA_ARGS__);	\
	}

IMPL_RUNNER_GET_COMPONENT(CommandCreator)
IMPL_COMPONENTS(IMPL_RUNNER_GET_COMPONENT)

Runner::Runner(sgrottel::ISimpleLog& log, commands::CommandFactory& factory)
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

	m_state = std::shared_ptr<lua_State>(luaL_newstate(), &lua_close);

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
	return m_components->m_CommandCreator.RegisterCommands();
}

bool Runner::LoadScript(const std::filesystem::path& script)
{
	if (!AssertStateReady()) return false;

	// script is assumed to be utf8 without BOM

	std::vector<char> data;

	FILE* file = nullptr;
	errno_t e = _wfopen_s(&file, script.wstring().c_str(), L"rb");
	if (e != 0 || file == nullptr)
	{
		m_log.Critical("Failed to load lua script: failed to open");
		return false;
	}

	{
		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		fseek(file, 0, SEEK_SET);

		data.resize(size);

		long size2 = static_cast<long>(fread(data.data(), 1, size, file));
		if (size2 != size)
		{
			m_log.Critical("Failed to read lua script: failed to read");
			return false;
		}
	}

	fclose(file);

	m_workingDirectory = script.parent_path();

	if (luaL_loadbuffer(m_state.get(), data.data(), data.size(), ""))
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

	std::filesystem::path oldCurrentPath = std::filesystem::current_path();
	bool retval = false;

	if (!m_workingDirectory.empty())
	{
		std::filesystem::current_path(m_workingDirectory);
	}

	try
	{
		if (lua_pcall(m_state.get(),
			0,	// no arguments to function
			0,	// no expected return values
			0	// default error function
		))
		{
			m_log.Critical("Failed to run lua script: %s", lua_tostring(m_state.get(), -1));
		}
		else
		{
			retval = true;
		}
	}
	catch (const std::exception& ex)
	{
		m_log.Critical("Exception while running lua script: %s", ex.what());
	}
	catch (...)
	{
		m_log.Critical("Unknown exception while running lua script");
	}

	if (!m_workingDirectory.empty())
	{
		std::filesystem::current_path(oldCurrentPath);
	}
	return retval;
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
