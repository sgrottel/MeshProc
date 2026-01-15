#include "IndexListType.h"

#include "lua/LuaUtilities.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool IndexListType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &IndexListType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &IndexListType::CallbackToString},
		{"__gc", &IndexListType::CallbackDelete},
		{"__len", &IndexListType::CallbackLength},
		{"__index", &IndexListType::CallbackGet},
		{"__newindex", &IndexListType::CallbackSet},
		{"insert", &IndexListType::CallbackInsert},
		{"remove", &IndexListType::CallbackRemove},
		{"resize", &IndexListType::CallbackResize},
		{nullptr, nullptr}
	};

	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");		// load global "meshproc"
	lua_newtable(lua());					// push new table on stack, which will become "meshproc.Mesh"
	luaL_setfuncs(lua(), staticFuncs, 0);	// Add static functions to new table
	lua_setfield(lua(), -2, "IndexList");	// store new table as "Mesh" in "meshproc"; also pops that table
	lua_pop(lua(), 1);						// remove "meshproc" from stack

	return true;
}

int IndexListType::CallbackCtor(lua_State* lua)
{
	IndexListType::LuaPush(lua, std::make_shared<std::vector<uint32_t>>());
	return 1;
}

int IndexListType::CallbackLength(lua_State* lua)
{
	// args of the "#" operator is twice the object instance
	const int argcnt = lua_gettop(lua);
	if (argcnt != 2)
	{
		// DumpLuaStack(lua);
		return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", argcnt);
	}
	// ignoring first
	const auto list = IndexListType::LuaGet(lua, 2);
	if (!list)
	{
		return luaL_error(lua, "Pre-First argument expected to be a IndexList");
	}

	lua_pushinteger(lua, list->size());
	return 1;
}

int IndexListType::CallbackGet(lua_State* lua)
{
	const int argcnt = lua_gettop(lua);
	if (argcnt != 2)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", argcnt);
	}

	const auto list = IndexListType::LuaGet(lua, 1);
	if (!list)
	{
		return luaL_error(lua, "Pre-First argument expected to be a IndexList");
	}

	uint32_t idx = 0;

	if (lua_isstring(lua, 2))
	{
		// try to load function callback
		size_t len;
		std::string name = luaL_tolstring(lua, 2, &len); // copy type string

		luaL_getmetatable(lua, LUA_TYPE_NAME);

		lua_getfield(lua, -1, name.c_str());

		if (lua_isnil(lua, -1))
		{
			lua_pop(lua, 2); // pop nil and metatable and try with string as index
		}
		else
		{
			lua_remove(lua, -2); // remove metatable, keep function pointer value
			return 1;
		}
	}

	auto res = GetLuaUint32(lua, 2, idx);
	if (res == GetResult::Ok)
	{
		// numeric index -> return list element
		if (idx == 0 || idx > list->size())
		{
			lua_pushnil(lua);
			return 1;
		}

		lua_pushinteger(lua, list->at(idx - 1) + 1);
		return 1;
	}

	return luaL_error(lua, "Failed to check agument %d", res);
}

int IndexListType::CallbackSet(lua_State* lua)
{
	const int argcnt = lua_gettop(lua);
	if (argcnt != 3)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 3, is %d", argcnt);
	}

	const auto list = IndexListType::LuaGet(lua, 1);
	if (!list)
	{
		return luaL_error(lua, "Pre-First argument expected to be a IndexList");
	}

	uint32_t idx;
	if (GetLuaUint32(lua, 2, idx) != GetResult::Ok)
	{
		return luaL_error(lua, "Failed to get insert index argument integer");
	}
	if (idx == 0 || idx > list->size())
	{
		return luaL_error(lua, "Invalid insert index argument integer, %d", idx);
	}

	uint32_t val;
	if (GetLuaUint32(lua, 3, val) != GetResult::Ok)
	{
		return luaL_error(lua, "Failed to get value argument");
	}

	list->at(idx - 1) = val - 1;
	return 0;
}

int IndexListType::CallbackInsert(lua_State* lua)
{
	const int argcnt = lua_gettop(lua);
	if ((argcnt != 2) && (argcnt != 3))
	{
		return luaL_error(lua, "Arguments number mismatch: must be 2 or 3, is %d", argcnt);
	}

	const auto list = IndexListType::LuaGet(lua, 1);
	if (!list)
	{
		return luaL_error(lua, "Pre-First argument expected to be a IndexList");
	}

	uint32_t val;
	if (GetLuaUint32(lua, -1, val) != GetResult::Ok)
	{
		return luaL_error(lua, "Failed to get value argument");
	}

	if (argcnt == 2)
	{
		list->push_back(val - 1);
		return 0;
	}

	uint32_t idx;
	if (GetLuaUint32(lua, 2, idx) != GetResult::Ok)
	{
		return luaL_error(lua, "Failed to get insert index argument integer");
	}
	if (idx == 0 || idx > list->size() + 1)
	{
		return luaL_error(lua, "Invalid insert index argument integer, %d", idx);
	}
	if (idx == list->size() + 1)
	{
		list->push_back(val - 1);
		return 0;
	}

	auto where = list->cbegin();
	std::advance(where, idx - 1);
	list->insert(where, val - 1);

	return 0;
}

int IndexListType::CallbackRemove(lua_State* lua)
{
	const int argcnt = lua_gettop(lua);
	if ((argcnt != 1) && (argcnt != 2))
	{
		return luaL_error(lua, "Arguments number mismatch: must be 1 or 2, is %d", argcnt);
	}

	const auto list = IndexListType::LuaGet(lua, 1);
	if (!list)
	{
		return luaL_error(lua, "Pre-First argument expected to be a IndexList");
	}

	if (argcnt == 1)
	{
		list->pop_back();
		return 0;
	}

	uint32_t idx;
	if (GetLuaUint32(lua, 2, idx) != GetResult::Ok)
	{
		return luaL_error(lua, "Failed to get insert index argument integer");
	}
	if (idx == 0 || idx > list->size() + 1)
	{
		return luaL_error(lua, "Invalid insert index argument integer, %d", idx);
	}

	auto where = list->cbegin();
	std::advance(where, idx - 1);
	list->erase(where);

	return 0;
}

int IndexListType::CallbackResize(lua_State* lua)
{
	const int argcnt = lua_gettop(lua);
	if (argcnt != 2)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", argcnt);
	}

	const auto list = IndexListType::LuaGet(lua, 1);
	if (!list)
	{
		return luaL_error(lua, "Pre-First argument expected to be a IndexList");
	}

	uint32_t newlen;
	if (GetLuaUint32(lua, 2, newlen) != GetResult::Ok)
	{
		return luaL_error(lua, "Failed to get insert index argument integer");
	}

	list->resize(newlen, -1);

	return 0;
}
