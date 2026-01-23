#include "FloatListType.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool FloatListType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &FloatListType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &FloatListType::CallbackToString},
		{"__gc", &FloatListType::CallbackDelete},
		{"__len", &FloatListType::CallbackLength},
		{"__index", &FloatListType::CallbackDispatchGet},
		{"__newindex", &FloatListType::CallbackSet},
		{"insert", &FloatListType::CallbackInsert},
		{"remove", &FloatListType::CallbackRemove},
		{"resize", &FloatListType::CallbackResize},
		{nullptr, nullptr}
	};

	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");
	lua_newtable(lua());
	luaL_setfuncs(lua(), staticFuncs, 0);
	lua_setfield(lua(), -2, "FloatList");
	lua_pop(lua(), 1);

	return true;
}

void FloatListType::LuaPushElementValue(lua_State* lua, const std::vector<float>& list, uint32_t indexZeroBased)
{
	lua_pushnumber(lua, list.at(indexZeroBased));
}

bool FloatListType::LuaGetElement(lua_State* lua, int i, float& outVal)
{
	if (GetLuaFloat(lua, i, outVal) == GetResult::Ok)
	{
		return true;
	}
	return false;
}

float FloatListType::GetInvalidValue()
{
	return 0.0f;
}
