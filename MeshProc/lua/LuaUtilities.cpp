#include "LuaUtilities.h"

#include <iostream>
#include <lua.hpp>

namespace
{

	void indent(int depth)
	{
		for (int i = 0; i < depth; ++i) std::cout << "  ";
	}

	void dump_value(lua_State* L, int index, int depth, const void* prevPtr = nullptr)
	{
		int type = lua_type(L, index);
		switch (type) {
		case LUA_TNUMBER:
			std::cout << lua_tonumber(L, index);
			break;
		case LUA_TSTRING:
			std::cout << "\"" << lua_tostring(L, index) << "\"";
			break;
		case LUA_TBOOLEAN:
			std::cout << (lua_toboolean(L, index) ? "true" : "false");
			break;
		case LUA_TTABLE:
		{
			const void* tabPtr = lua_topointer(L, index);
			std::cout << "table @" << tabPtr;
			if (prevPtr != tabPtr)
			{
				std::cout << " {\n";
				lua_pushnil(L);  // first key
				while (lua_next(L, index < 0 ? index - 1 : index))
				{
					indent(depth + 1);
					std::cout << "[";
					dump_value(L, -2, depth + 1);  // key
					std::cout << "] = ";
					dump_value(L, -1, depth + 1, tabPtr);  // value
					std::cout << "\n";
					lua_pop(L, 1);  // remove value, keep key for next iteration
				}
				indent(depth);
				std::cout << "}";
			}
			else
			{
				std::cout << "<recursion>";
			}
		}
			break;
		case LUA_TUSERDATA:
		{
			std::string name{ "error" };
			if (lua_getmetatable(L, index))
			{
				void* mt_ptr = (void*)lua_topointer(L, -1);
				lua_pushnil(L); // first key for lua_next
				while (lua_next(L, LUA_REGISTRYINDEX) != 0)
				{
					if (lua_topointer(L, -1) == mt_ptr && lua_type(L, -2) == LUA_TSTRING)
					{
						name = lua_tostring(L, -2);
						lua_pop(L, 2); // pop key + value
						break;
					}
					lua_pop(L, 1); // pop value
				}
				lua_pop(L, 1); // pop mt
			}
			std::cout << "userdata \"" << name << "\" @" << lua_topointer(L, index);
		}
			break;
		default:
			std::cout << lua_typename(L, type) << " @" << lua_topointer(L, index);
			break;
		}
	}

}

void meshproc::lua::DumpLuaStack(lua_State* L)
{
	int top = lua_gettop(L);
	std::cout << "Lua stack (top -> bottom):\n";
	for (int i = top; i >= 1; --i)
	{
		indent(1);
		std::cout << i << ": ";
		dump_value(L, i, 1);
		std::cout << "\n";
	}
}

meshproc::lua::GetResult meshproc::lua::GetLuaUint32(lua_State* lua, int i, uint32_t& tar)
{
	if (lua == nullptr)
	{
		return GetResult::ErrorState;
	}

	if (lua_isnumber(lua, i))
	{
		const auto number = lua_tonumber(lua, i);
		if (number < 0.0) return GetResult::ErrorValue;

		tar = static_cast<uint32_t>(number);
		return GetResult::Ok;
	}

	if (lua_isinteger(lua, i))
	{
		const auto integer = lua_tointeger(lua, i);
		if (integer < 0) return GetResult::ErrorValue;

		tar = static_cast<uint32_t>(integer);
		return GetResult::Ok;
	}

	const int size = lua_gettop(lua);
	if (i == 0
		|| (i > 0 && size < i)
		|| (i < 0 && size < -i))
	{
		return GetResult::ErrorIndex;
	}

	return GetResult::ErrorType;
}

meshproc::lua::GetResult meshproc::lua::GetLuaFloat(lua_State* lua, int i, float& tar)
{
	if (lua == nullptr)
	{
		return GetResult::ErrorState;
	}

	if (lua_isnumber(lua, i))
	{
		tar = static_cast<float>(lua_tonumber(lua, i));
		return GetResult::Ok;
	}
	if (lua_isinteger(lua, i))
	{
		tar = static_cast<float>(lua_tointeger(lua, i));
		return GetResult::Ok;
	}

	const int size = lua_gettop(lua);
	if (i == 0
		|| (i > 0 && size < i)
		|| (i < 0 && size < -i))
	{
		return GetResult::ErrorIndex;
	}

	return GetResult::ErrorType;
}
