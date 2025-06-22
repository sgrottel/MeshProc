#include "GlmVec2Type.h"

#include "LuaUtilities.h"

#include <lua.hpp>

#include <glm/gtc/type_ptr.hpp>

using namespace meshproc;
using namespace meshproc::lua;

bool GlmVec2Type::Push(lua_State* lua, const glm::vec2& val)
{
	lua_createtable(lua, 0, 2); // Preallocate array part
	lua_pushnumber(lua, val.x);
	lua_setfield(lua, -2, "x");
	lua_pushnumber(lua, val.y);
	lua_setfield(lua, -2, "y");

	// Step 2: Set the XVec3 metatable
	lua_getfield(lua, LUA_REGISTRYINDEX, "XVec2_mt");
	if (!lua_istable(lua, -1))
	{
		lua_pop(lua, 2); // cleanup table and non-table
		luaL_error(lua, "XVec2_mt not found in registry. Is xyz_math loaded?");
		return false;
	}
	lua_setmetatable(lua, -2); // Set metatable on the matrix table

	return true;
}

bool GlmVec2Type::TryGet(lua_State* lua, int idx, glm::vec2& tar)
{
	luaL_checktype(lua, idx, LUA_TTABLE);
	int checkVecType = 0;
	do {
		if (!lua_getmetatable(lua, idx))
		{
			return luaL_error(lua, "Expected XVec2, but argument has no metatable");
		}
		lua_getfield(lua, LUA_REGISTRYINDEX, "XVec2_mt");  // Push expected metatable

		if (lua_rawequal(lua, -1, -2))
		{
			checkVecType = 2;
			break;
		}

		lua_pop(lua, 1);
		lua_getfield(lua, LUA_REGISTRYINDEX, "XVec3_mt");  // Push expected metatable

		if (lua_rawequal(lua, -1, -2))
		{
			checkVecType = 3;
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

	float x, y;

	lua_getfield(lua, idx, "x");
	x = (float)luaL_checknumber(lua, -1);
	lua_pop(lua, 1);

	lua_getfield(lua, idx, "y");
	y = (float)luaL_checknumber(lua, -1);
	lua_pop(lua, 1);

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
		}
	}

	tar = glm::vec2{ x, y };

	return true;
}
