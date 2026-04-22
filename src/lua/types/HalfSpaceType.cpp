#include "HalfSpaceType.h"

#include "data/HalfSpace.h"

#include "GlmVec3Type.h"
#include "lua/LuaUtilities.h"

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool HalfSpaceType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &HalfSpaceType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &HalfSpaceType::CallbackToString},
		{"__gc", &HalfSpaceType::CallbackDelete},
		{"set", &HalfSpaceType::CallbackSet},
		{"get", &HalfSpaceType::CallbackGet},
		{"dist", &HalfSpaceType::CallbackDist},
		{nullptr, nullptr}
	};

	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");
	lua_newtable(lua());
	luaL_setfuncs(lua(), staticFuncs, 0);
	lua_setfield(lua(), -2, "HalfSpace");
	lua_pop(lua(), 1);

	return true;
}

int HalfSpaceType::CallbackCtor(lua_State* lua)
{
	HalfSpaceType::LuaPush(lua, std::make_shared<data::HalfSpace>());
	return 1;
}

int HalfSpaceType::CallbackSet(lua_State* lua)
{
	// DumpLuaStack(lua);
	int size = lua_gettop(lua);
	if (size != 3)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 3, is %d", size);
	}
	auto hs = HalfSpaceType::LuaGet(lua, 1);
	if (!hs)
	{
		return luaL_error(lua, "Pre-First argument expected to be a HalfSpace");
	}

	glm::vec3 n;
	if (!GlmVec3Type::TryGet(lua, 2, n))
	{
		return luaL_error(lua, "First argument expected to be a vec3");
	}
	if (n.x == 0 && n.y == 0 && n.z == 0)
	{
		return luaL_error(lua, "First argument normal vector must not be zero");
	}

	float d;
	auto const r = GetLuaFloat(lua, 3, d);
	if (r == GetResult::Ok)
	{
		hs->Set(n, d);
		return 0;
	}
	if (r == GetResult::ErrorType)
	{
		glm::vec3 p;
		if (GlmVec3Type::TryGet(lua, 3, p))
		{
			hs->Set(n, p);
			return 0;
		}
	}

	return luaL_error(lua, "Second argument expected to be a vec3 (point) or float (dist)s");
}

int HalfSpaceType::CallbackGet(lua_State* lua)
{
	// DumpLuaStack(lua);
	int size = lua_gettop(lua);
	if (size != 1)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 1, is %d", size);
	}
	auto hs = HalfSpaceType::LuaGet(lua, 1);
	if (!hs)
	{
		return luaL_error(lua, "Pre-First argument expected to be a HalfSpace");
	}

	GlmVec3Type::Push(lua, hs->Normal());
	lua_pushnumber(lua, hs->Dist());

	return 2;
}

int HalfSpaceType::CallbackDist(lua_State* lua)
{
	// DumpLuaStack(lua);
	int size = lua_gettop(lua);
	if (size != 2)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", size);
	}
	auto hs = HalfSpaceType::LuaGet(lua, 1);
	if (!hs)
	{
		return luaL_error(lua, "Pre-First argument expected to be a HalfSpace");
	}

	glm::vec3 p;
	if (!GlmVec3Type::TryGet(lua, 2, p))
	{
		return luaL_error(lua, "First argument expected to be a vec3");
	}

	lua_pushnumber(lua, hs->Dist(p));
	return 1;
}
