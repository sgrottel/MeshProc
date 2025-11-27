#include "ListOfVec3Type.h"

#include "GlmVec3Type.h"
#include "LuaUtilities.h"

using namespace meshproc;
using namespace meshproc::lua;

bool ListOfVec3Type::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &ListOfVec3Type::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &ListOfVec3Type::CallbackToString},
		{"__gc", &ListOfVec3Type::CallbackDelete},
		{"size", &ListOfVec3Type::CallbackSize},
		{"get", &ListOfVec3Type::CallbackGet},
		{"set", &ListOfVec3Type::CallbackSet},
		{nullptr, nullptr}
	};
	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");		// load global "meshproc"
	lua_newtable(lua());					// push new table on stack, which will become "meshproc.ListOfVec3"
	luaL_setfuncs(lua(), staticFuncs, 0);	// Add static functions to new table
	lua_setfield(lua(), -2, "ListOfVec3");	// store new table as "ListOfVec3" in "meshproc"; also pops that table
	lua_pop(lua(), 1);						// remove "meshproc" from stack

	return true;
}

int ListOfVec3Type::CallbackCtor(lua_State* lua)
{
	int size = lua_gettop(lua);
	if (size != 1)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 1, is %d", size);
	}
	uint32_t len;
	if (GetResult::Ok != GetLuaUint32(lua, 1, len))
	{
		return luaL_error(lua, "First argument expected to be an integer");
	}

	auto data = std::make_shared<std::vector<glm::vec3>>();
	data->resize(len, glm::vec3{ 0.0f, 0.0f, 0.0f });
	ListOfVec3Type::LuaPush(lua, data);
	return 1;
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

int ListOfVec3Type::CallbackSet(lua_State* lua)
{
	int size = lua_gettop(lua);
	if (size != 3)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 3, is %d", size);
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

	glm::vec3 v;
	if (!GlmVec3Type::TryGet(lua, 3, v))
	{
		return luaL_error(lua, "Second argument expected to be a vec3");
	}

	val->at(idx - 1) = v;

	return 0;
}
