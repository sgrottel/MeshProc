#include "LuaUtilities.h"

#include <iostream>
#include <lua.hpp>

namespace
{

	void indent(int depth) {
		for (int i = 0; i < depth; ++i) std::cout << "  ";
	}

	void dump_value(lua_State* L, int index, int depth, const void* prevPtr = nullptr) {
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
				while (lua_next(L, index < 0 ? index - 1 : index)) {
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
		default:
			std::cout << lua_typename(L, type) << " @" << lua_topointer(L, index);
			break;
		}
	}

}

void meshproc::lua::DumpLuaStack(lua_State* L) {
	int top = lua_gettop(L);
	std::cout << "Lua stack (top -> bottom):\n";
	for (int i = top; i >= 1; --i) {
		indent(1);
		std::cout << i << ": ";
		dump_value(L, i, 1);
		std::cout << "\n";
	}
}
