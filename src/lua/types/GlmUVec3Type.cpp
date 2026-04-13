#include "GlmUVec3Type.h"

#include "lua/LuaUtilities.h"

#include <lua.hpp>

#include <glm/gtc/type_ptr.hpp>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool GlmUVec3Type::Push(lua_State* lua, const glm::uvec3& val)
{
	lua_createtable(lua, 0, 3); // Preallocate array part
	lua_pushinteger(lua, val.x);
	lua_setfield(lua, -2, "x");
	lua_pushinteger(lua, val.y);
	lua_setfield(lua, -2, "y");
	lua_pushinteger(lua, val.z);
	lua_setfield(lua, -2, "z");

	// Step 2: Set the XVec3 metatable
	lua_getfield(lua, LUA_REGISTRYINDEX, "XVec3_mt");
	if (!lua_istable(lua, -1))
	{
		lua_pop(lua, 2); // cleanup table and non-table
		luaL_error(lua, "XVec3_mt not found in registry. Is xyz_math loaded?");
		return false;
	}
	lua_setmetatable(lua, -2); // Set metatable on the matrix table

	return true;
}

bool GlmUVec3Type::TryGet(lua_State * lua, int idx, glm::uvec3 & tar)
{
	luaL_checktype(lua, idx, LUA_TTABLE);

	if (!lua_getmetatable(lua, idx))
	{
		return luaL_error(lua, "Expected XVec3, but argument has no metatable");
	}
	lua_getfield(lua, LUA_REGISTRYINDEX, "XVec3_mt");  // Push expected metatable

	const bool typeMatch = lua_rawequal(lua, -1, -2);
	lua_pop(lua, 2);
	if (!typeMatch)
	{
		return luaL_error(lua, "Expected XVec3, but argument has unknown metatable");
	}

	lua_getfield(lua, idx, "x");
	auto succ = lua::GetLuaUint32(lua, -1, tar.x);
	lua_pop(lua, 1);
	if (succ != GetResult::Ok)
	{
		return luaL_error(lua, "Failed to read x value as integer");
	}

	lua_getfield(lua, idx, "y");
	succ = lua::GetLuaUint32(lua, -1, tar.y);
	lua_pop(lua, 1);
	if (succ != GetResult::Ok)
	{
		return luaL_error(lua, "Failed to read y value as integer");
	}

	lua_getfield(lua, idx, "z");
	succ = lua::GetLuaUint32(lua, -1, tar.z);
	lua_pop(lua, 1);
	if (succ != GetResult::Ok)
	{
		return luaL_error(lua, "Failed to read z value as integer");
	}

	return true;
}
