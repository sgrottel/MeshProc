#include "VersionCheck.h"

#include "LuaUtilities.h"
#include "VersionInfo.h"

#include <SimpleLog/SimpleLog.hpp>

#include <lua.hpp>

#include <algorithm>

using namespace meshproc;
using namespace meshproc::lua;

namespace
{
	constexpr char const* MeshProcVersionNumber = "MeshProcVersionNumber";
}

bool VersionCheck::Init()
{
	if (!AssertStateReady()) return false;

	luaL_newmetatable(lua(), MeshProcVersionNumber);
	lua_pushstring(lua(), "__index");
	lua_pushvalue(lua(), -2);
	lua_settable(lua(), -3);

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &VersionCheck::CallbackToString},
		{nullptr, nullptr}
	};

	luaL_setfuncs(lua(), memberFuncs, 0);
	lua_pop(lua(), 1);

	lua_newtable(lua());

	lua_pushcfunction(lua(), &VersionCheck::CallbackGetVersion);
	lua_setfield(lua(), -2, "getVersion");

	lua_pushcfunction(lua(), &VersionCheck::CallbackAssertVersion);
	lua_setfield(lua(), -2, "assert");
	lua_pushcfunction(lua(), &VersionCheck::CallbackAssertVersionSince);
	lua_setfield(lua(), -2, "assertNewerThan");
	lua_pushcfunction(lua(), &VersionCheck::CallbackAssertVersionSinceIncluded);
	lua_setfield(lua(), -2, "assertOrNewer");
	lua_pushcfunction(lua(), &VersionCheck::CallbackAssertVersionBefore);
	lua_setfield(lua(), -2, "assertOlderThan");
	lua_pushcfunction(lua(), &VersionCheck::CallbackAssertVersionBeforeIncluded);
	lua_setfield(lua(), -2, "assertOrOlder");

	lua_setglobal(lua(), "meshprocVersion");

	return true;
}

int VersionCheck::CallbackToString(lua_State* lua)
{
	luaL_checktype(lua, 1, LUA_TTABLE);

	if (!lua_getmetatable(lua, 1))
	{
		return luaL_error(lua, "Expected MeshProcVersionNumber, but argument has no metatable");
	}
	lua_getfield(lua, LUA_REGISTRYINDEX, MeshProcVersionNumber);
	if (!lua_rawequal(lua, -1, -2))
	{
		lua_pop(lua, 2);
		return luaL_error(lua, "Expected MeshProcVersionNumber, but argument has different metatable");
	}
	lua_pop(lua, 2);

	auto tableLen = lua_rawlen(lua, 1);
	std::string str = "";
	for (int i = 0; i < std::min<int>(static_cast<int>(tableLen), 4); ++i)
	{
		lua_rawgeti(lua, 1, i + 1);

		uint32_t v;
		if (GetLuaUint32(lua, -1, v) != GetResult::Ok)
		{
			return luaL_error(lua, "Expected MeshProcVersionNumber entry %d to be a number", i + 1);
		}

		if (i > 0) str += ".";
		str += std::to_string(v);

		lua_pop(lua, 1);
	}
	for (int i = 4; i < tableLen; ++i)
	{
		lua_rawgeti(lua, 1, i + 1);
		str += ".";
		str += lua_tostring(lua, -1);
		lua_pop(lua, 1);
	}

	lua_pushstring(lua, str.c_str());
	return 1;
}

int VersionCheck::CallbackGetVersion(lua_State* lua)
{
	lua_createtable(lua, 4, 0);

	lua_pushnumber(lua, MESHPROC_VER_MAJOR);
	lua_rawseti(lua, -2, 1);
	lua_pushnumber(lua, MESHPROC_VER_MINOR);
	lua_rawseti(lua, -2, 2);
	lua_pushnumber(lua, MESHPROC_VER_PATCH);
	lua_rawseti(lua, -2, 3);
	lua_pushnumber(lua, MESHPROC_VER_BUILD);
	lua_rawseti(lua, -2, 4);

	lua_getfield(lua, LUA_REGISTRYINDEX, MeshProcVersionNumber);
	if (!lua_istable(lua, -1))
	{
		luaL_error(lua, "MeshProcVersionNumber not found in registry.");
		return false;
	}
	lua_setmetatable(lua, -2); // Set metatable on the matrix table

	return 1;
}

int VersionCheck::CallbackAssertVersion(lua_State* lua)
{
	int cmp = GetVersionParamsCompare(lua);
	if (cmp < -1 || cmp > 1)
	{
		luaL_error(lua, "Failed to parse assert arguments as version information");
		return 0;
	}
	if (cmp < 0)
	{
		luaL_error(lua, "The script requires an older version of MeshProc to be used");
		return 0;
	}
	if (cmp > 0)
	{
		luaL_error(lua, "The script requires a newer version of MeshProc to be used");
		return 0;
	}
	return 0;
}

int VersionCheck::CallbackAssertVersionSince(lua_State* lua)
{
	int cmp = GetVersionParamsCompare(lua);
	if (cmp < -1 || cmp > 1)
	{
		luaL_error(lua, "Failed to parse assert arguments as version information");
		return 0;
	}
	if (cmp >= 0)
	{
		luaL_error(lua, "The script requires a newer version of MeshProc to be used");
		return 0;
	}
	return 0;
}

int VersionCheck::CallbackAssertVersionSinceIncluded(lua_State* lua)
{
	int cmp = GetVersionParamsCompare(lua);
	if (cmp < -1 || cmp > 1)
	{
		luaL_error(lua, "Failed to parse assert arguments as version information");
		return 0;
	}
	if (cmp > 0)
	{
		luaL_error(lua, "The script requires a newer version of MeshProc to be used");
		return 0;
	}
	return 0;
}

int VersionCheck::CallbackAssertVersionBefore(lua_State* lua)
{
	int cmp = GetVersionParamsCompare(lua);
	if (cmp < -1 || cmp > 1)
	{
		luaL_error(lua, "Failed to parse assert arguments as version information");
		return 0;
	}
	if (cmp <= 0)
	{
		luaL_error(lua, "The script requires an older version of MeshProc to be used");
		return 0;
	}
	return 0;
}

int VersionCheck::CallbackAssertVersionBeforeIncluded(lua_State* lua)
{
	int cmp = GetVersionParamsCompare(lua);
	if (cmp < -1 || cmp > 1)
	{
		luaL_error(lua, "Failed to parse assert arguments as version information");
		return 0;
	}
	if (cmp < 0)
	{
		luaL_error(lua, "The script requires an older version of MeshProc to be used");
		return 0;
	}
	return 0;
}

int VersionCheck::GetVersionParams(lua_State* lua, uint32_t& major, uint32_t& minor, uint32_t& patch, uint32_t& build)
{
	int nargs = lua_gettop(lua);
	if (nargs > 4)
	{
		luaL_error(lua, "Failed to parse version assert parameter list: too many arguments");
		return -1;
	}

	for (int i = 0; i < nargs; ++i)
	{
		uint32_t v;
		if (GetLuaUint32(lua, 1 + i, v) != GetResult::Ok)
		{
			luaL_error(lua, "Failed to parse version assert parameter list: expected param %d to be a number", i + 1);
			return -1;
		}
		switch (i)
		{
		case 0: major = v; break;
		case 1: minor = v; break;
		case 2: patch = v; break;
		case 3: build = v; break;
		}
	}
	return nargs;
}

int VersionCheck::GetVersionParamsCompare(lua_State* lua)
{
	uint32_t major, minor, patch, build;
	int verLen = GetVersionParams(lua, major, minor, patch, build);

	if (verLen < 0) return -3; // parse error
	if (verLen == 0) return -2; // no data

	if (major < MESHPROC_VER_MAJOR) return -1; // specified version is smaller/older than the version of this assembly
	if (major > MESHPROC_VER_MAJOR) return 1; // specified version is larger/newer than the version of this assembly
	if (verLen == 1) return 0; // versions equal enough

	if (minor < MESHPROC_VER_MINOR) return -1; // specified version is smaller/older than the version of this assembly
	if (minor > MESHPROC_VER_MINOR) return 1; // specified version is larger/newer than the version of this assembly
	if (verLen == 2) return 0; // versions equal enough

	if (patch < MESHPROC_VER_PATCH) return -1; // specified version is smaller/older than the version of this assembly
	if (patch > MESHPROC_VER_PATCH) return 1; // specified version is larger/newer than the version of this assembly
	if (verLen == 3) return 0; // versions equal enough

	if (build < MESHPROC_VER_BUILD) return -1; // specified version is smaller/older than the version of this assembly
	if (build > MESHPROC_VER_BUILD) return 1; // specified version is larger/newer than the version of this assembly

	return 0;
}
