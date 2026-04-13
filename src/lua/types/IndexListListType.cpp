#include "IndexListListType.h"

#include "IndexListType.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool IndexListListType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &IndexListListType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &IndexListListType::CallbackToString},
		{"__gc", &IndexListListType::CallbackDelete},
		{"__len", &IndexListListType::CallbackLength},
		{"__index", &IndexListListType::CallbackDispatchGet},
		{"__newindex", &IndexListListType::CallbackSet},
		{"insert", &IndexListListType::CallbackInsert},
		{"remove", &IndexListListType::CallbackRemove},
		{"resize", &IndexListListType::CallbackResize},
		{nullptr, nullptr}
	};

	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");		// load global "meshproc"
	lua_newtable(lua());					// push new table on stack, which will become "meshproc.Mesh"
	luaL_setfuncs(lua(), staticFuncs, 0);	// Add static functions to new table
	lua_setfield(lua(), -2, "IndexListList");// store new table as "IndexListList" in "meshproc"; also pops that table
	lua_pop(lua(), 1);						// remove "meshproc" from stack

	return true;
}

void IndexListListType::LuaPushElementValue(lua_State* lua, const std::vector<IndexListListValueType>& list, uint32_t indexZeroBased)
{
	IndexListType::LuaPush(lua, list.at(indexZeroBased));
}

bool IndexListListType::LuaGetElement(lua_State* lua, int i, IndexListListValueType& outVal)
{
	outVal = IndexListType::LuaGet(lua, i);
	return true;
}

IndexListListValueType IndexListListType::GetInvalidValue()
{
	return nullptr;
}
