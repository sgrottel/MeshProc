#include "IndicesType.h"

#include "LuaUtilities.h"

using namespace meshproc;
using namespace meshproc::lua;

bool IndicesType::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &IndicesType::CallbackToString},
		{"__gc", &IndicesType::CallbackDelete},
		{"size", &IndicesType::CallbackSize},
		{"get", &IndicesType::CallbackGet},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}

int IndicesType::CallbackSize(lua_State* lua)
{
	int size = lua_gettop(lua);
	if (size != 1)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 1, is %d", size);
	}
	auto val = LuaGet(lua, 1);
	if (!val)
	{
		return luaL_error(lua, "Pre-First argument expected to be a %s", LUA_TYPE_NAME);
	}

	lua_pushinteger(lua, val->size());
	return 1;
}

int IndicesType::CallbackGet(lua_State* lua)
{
	int size = lua_gettop(lua);
	if (size != 2)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", size);
	}
	auto val = LuaGet(lua, 1);
	if (!val)
	{
		return luaL_error(lua, "Pre-First argument expected to be a %s", LUA_TYPE_NAME);
	}
	uint32_t idx;
	if (GetResult::Ok != GetLuaUint32(lua, 2, idx))
	{
		return luaL_error(lua, "First argument expected to be an integer");
	}
	if (idx == 0 || idx > val->size())
	{
		return luaL_error(lua, "Argument out of bounds expected [1..%d], got %d", static_cast<int>(val->size()), static_cast<int>(idx));
	}

	lua_pushinteger(lua, val->at(idx - 1));

	return 1;
}
