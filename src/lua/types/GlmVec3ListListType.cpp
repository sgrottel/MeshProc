#include "GlmVec3ListListType.h"

#include "GlmVec3ListType.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool GlmVec3ListListType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &GlmVec3ListListType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &GlmVec3ListListType::CallbackToString},
		{"__gc", &GlmVec3ListListType::CallbackDelete},
		{"__len", &GlmVec3ListListType::CallbackLength},
		{"__index", &GlmVec3ListListType::CallbackDispatchGet},
		{"__newindex", &GlmVec3ListListType::CallbackSet},
		{"insert", &GlmVec3ListListType::CallbackInsert},
		{"remove", &GlmVec3ListListType::CallbackRemove},
		{"resize", &GlmVec3ListListType::CallbackResize},
		{nullptr, nullptr}
	};

	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");
	lua_newtable(lua());
	luaL_setfuncs(lua(), staticFuncs, 0);
	lua_setfield(lua(), -2, "Vec3ListList");
	lua_pop(lua(), 1);

	return true;
}

void GlmVec3ListListType::LuaPushElementValue(lua_State* lua, const std::vector<GlmVec3ListListValueType>& list, uint32_t indexZeroBased)
{
	GlmVec3ListType::LuaPush(lua, list.at(indexZeroBased));
}

bool GlmVec3ListListType::LuaGetElement(lua_State* lua, int i, GlmVec3ListListValueType& outVal)
{
	outVal = GlmVec3ListType::LuaGet(lua, i);
	return true;
}

GlmVec3ListListValueType GlmVec3ListListType::GetInvalidValue()
{
	return nullptr;
}
