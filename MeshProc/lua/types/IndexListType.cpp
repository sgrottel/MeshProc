#include "IndexListType.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool IndexListType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &IndexListType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &IndexListType::CallbackToString},
		{"__gc", &IndexListType::CallbackDelete},
		{"__len", &IndexListType::CallbackLength},
		{"__index", &IndexListType::CallbackDispatchGet},
		{"__newindex", &IndexListType::CallbackSet},
		{"insert", &IndexListType::CallbackInsert},
		{"remove", &IndexListType::CallbackRemove},
		{"resize", &IndexListType::CallbackResize},
		{nullptr, nullptr}
	};

	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");		// load global "meshproc"
	lua_newtable(lua());					// push new table on stack, which will become "meshproc.Mesh"
	luaL_setfuncs(lua(), staticFuncs, 0);	// Add static functions to new table
	lua_setfield(lua(), -2, "IndexList");	// store new table as "Mesh" in "meshproc"; also pops that table
	lua_pop(lua(), 1);						// remove "meshproc" from stack

	return true;
}

void IndexListType::LuaPushElementValue(lua_State* lua, const std::vector<uint32_t>& list, uint32_t indexZeroBased)
{
	lua_pushinteger(lua, list.at(indexZeroBased) + 1);
}

bool IndexListType::LuaGetElement(lua_State* lua, int i, uint32_t& outVal)
{
	if (GetLuaUint32(lua, i, outVal) == GetResult::Ok)
	{
		outVal--;
		return true;
	}
	return false;
}

uint32_t IndexListType::GetInvalidValue()
{
	return -1;
}
