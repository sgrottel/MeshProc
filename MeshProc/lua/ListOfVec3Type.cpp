#include "ListOfVec3Type.h"

#include "GlmVec3Type.h"
#include "LuaUtilities.h"

using namespace meshproc;
using namespace meshproc::lua;

bool ListOfVec3Type::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &ListOfVec3Type::CallbackToString},
		{"__gc", &ListOfVec3Type::CallbackDelete},
		{"size", &ListOfVec3Type::CallbackSize},
		{"get", &ListOfVec3Type::CallbackGet},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}

int ListOfVec3Type::CallbackSize(lua_State* lua)
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

int ListOfVec3Type::CallbackGet(lua_State* lua)
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

	GlmVec3Type::Push(lua, val->at(idx - 1));
	return 1;
}
