#include "MeshListType.h"

#include "MeshType.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool MeshListType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &MeshListType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &MeshListType::CallbackToString},
		{"__gc", &MeshListType::CallbackDelete},
		{"__len", &MeshListType::CallbackLength},
		{"__index", &MeshListType::CallbackDispatchGet},
		{"__newindex", &MeshListType::CallbackSet},
		{"insert", &MeshListType::CallbackInsert},
		{"remove", &MeshListType::CallbackRemove},
		{"resize", &MeshListType::CallbackResize},
		{nullptr, nullptr}
	};

	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");
	lua_newtable(lua());
	luaL_setfuncs(lua(), staticFuncs, 0);
	lua_setfield(lua(), -2, "MeshList");
	lua_pop(lua(), 1);

	return true;
}

void MeshListType::LuaPushElementValue(lua_State* lua, const std::vector<MeshListValueType>& list, uint32_t indexZeroBased)
{
	MeshType::LuaPush(lua, list.at(indexZeroBased));
}

bool MeshListType::LuaGetElement(lua_State* lua, int i, MeshListValueType& outVal)
{
	outVal = MeshType::LuaGet(lua, i);
	return true;
}

MeshListValueType MeshListType::GetInvalidValue()
{
	return nullptr;
}
