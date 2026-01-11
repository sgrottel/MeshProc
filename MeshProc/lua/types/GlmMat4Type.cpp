#include "GlmMat4Type.h"

#include "lua/LuaUtilities.h"

#include <lua.hpp>

#include <glm/gtc/type_ptr.hpp>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool GlmMat4Type::Push(lua_State* lua, const glm::mat4& val)
{
	glm::mat4 m = glm::transpose(val);
	auto values = glm::value_ptr(m);

	// Step 1: Create the table
	lua_createtable(lua, 16, 0); // Preallocate array part with 16 elements
	for (int i = 0; i < 16; ++i)
	{
		lua_pushnumber(lua, values[i]);
		lua_rawseti(lua, -2, i + 1); // Lua is 1-indexed
	}

	// Step 2: Set the XMat4 metatable
	lua_getfield(lua, LUA_REGISTRYINDEX, "XMat4_mt");
	if (!lua_istable(lua, -1))
	{
		lua_pop(lua, 2); // cleanup table and non-table
		luaL_error(lua, "XMat4_mt not found in registry. Is xyz_math loaded?");
		return false;
	}
	lua_setmetatable(lua, -2); // Set metatable on the matrix table

	return true;
}

bool GlmMat4Type::TryGet(lua_State* lua, int idx, glm::mat4& tar)
{
	luaL_checktype(lua, idx, LUA_TTABLE);

	auto tableLen = lua_rawlen(lua, idx);
	if (tableLen == 16)
	{
		// assume 4x4 matrix
		lua_getfield(lua, LUA_REGISTRYINDEX, "XMat4_mt"); // push expected metatable
		if (!lua_getmetatable(lua, idx)) // push actual metatable of value
		{
			return luaL_error(lua, "Expected XMat4, but value has no metatable");
		}
		if (!lua_rawequal(lua, -1, -2))
		{
			return luaL_error(lua, "Expected XMat4, incorrect metatable");
		}
		lua_pop(lua, 2); // pop both metatables

		// Extract matrix
		glm::mat4 m;
		float* mat = glm::value_ptr(m);
		for (int i = 0; i < 16; ++i)
		{
			lua_rawgeti(lua, idx, i + 1);
			mat[i] = (float)luaL_checknumber(lua, -1);
			lua_pop(lua, 1);
		}
		tar = glm::transpose(m);
		return true;

	}
	else if (tableLen == 9)
	{
		// assume 3x3 matrix => implicit cast to Mat4
		lua_getfield(lua, LUA_REGISTRYINDEX, "XMat3_mt"); // push expected metatable
		if (!lua_getmetatable(lua, idx)) // push actual metatable of value
		{
			return luaL_error(lua, "Expected XMat4, but value has no metatable");
		}
		if (!lua_rawequal(lua, -1, -2))
		{
			return luaL_error(lua, "Expected XMat4, incorrect metatable");
		}
		lua_pop(lua, 2); // pop both metatables

		// Extract matrix
		glm::mat3 m;
		float* mat = glm::value_ptr(m);
		for (int i = 0; i < 9; ++i)
		{
			lua_rawgeti(lua, idx, i + 1);
			mat[i] = (float)luaL_checknumber(lua, -1);
			lua_pop(lua, 1);
		}
		tar = glm::transpose(m);
		return true;

	}
	else
	{
		return luaL_error(lua, "Expected XMat4, but value has wrong size");
	}

	return false;
}
