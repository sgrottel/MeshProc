#include "GlmVec3Type.h"

#include "lua/LuaUtilities.h"

#include <lua.hpp>

#include <glm/gtc/type_ptr.hpp>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool GlmVec3Type::Push(lua_State* lua, const glm::vec3& val)
{
	lua_createtable(lua, 0, 3); // Preallocate array part
	lua_pushnumber(lua, val.x);
	lua_setfield(lua, -2, "x");
	lua_pushnumber(lua, val.y);
	lua_setfield(lua, -2, "y");
	lua_pushnumber(lua, val.z);
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

bool GlmVec3Type::TryGet(lua_State * lua, int idx, glm::vec3 & tar)
{
	luaL_checktype(lua, idx, LUA_TTABLE);
	int checkVecType = 0;
	do {
		if (!lua_getmetatable(lua, idx))
		{
			return luaL_error(lua, "Expected XVec3, but argument has no metatable");
		}
		lua_getfield(lua, LUA_REGISTRYINDEX, "XVec3_mt");  // Push expected metatable

		if (lua_rawequal(lua, -1, -2))
		{
			checkVecType = 3;
			break;
		}

		lua_pop(lua, 1);
		lua_getfield(lua, LUA_REGISTRYINDEX, "XVec2_mt");  // Push expected metatable

		if (lua_rawequal(lua, -1, -2))
		{
			checkVecType = 2;
			break;
		}

		lua_pop(lua, 1);
		lua_getfield(lua, LUA_REGISTRYINDEX, "XVec4_mt");  // Push expected metatable

		if (lua_rawequal(lua, -1, -2))
		{
			checkVecType = 4;
		}
	} while (false);
	lua_pop(lua, 2);
	if (checkVecType == 0)
	{
		return luaL_error(lua, "Expected XVec3, but argument has unknown metatable");
	}

	float x, y, z{ 0.0f };

	lua_getfield(lua, idx, "x");
	x = (float)luaL_checknumber(lua, -1);
	lua_pop(lua, 1);

	lua_getfield(lua, idx, "y");
	y = (float)luaL_checknumber(lua, -1);
	lua_pop(lua, 1);

	if (checkVecType > 2)
	{
		lua_getfield(lua, idx, "z");
		z = (float)luaL_checknumber(lua, -1);
		lua_pop(lua, 1);
	}

	if (checkVecType == 4)
	{
		lua_getfield(lua, idx, "w");
		float w = (float)luaL_checknumber(lua, -1);
		lua_pop(lua, 1);

		if (std::abs(w) < 0.00001f || std::abs(w - 1.0) < 0.00001f)
		{
			// 'w' is either zero or one -> ignore w
		}
		else
		{
			// 'w' is a number, do perspective divide
			x /= w;
			y /= w;
			z /= w;
		}
	}

	tar = glm::vec3{ x, y, z };

	return true;
}
