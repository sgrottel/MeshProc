#include "GlmVec3ListType.h"

#include "GlmVec3Type.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool GlmVec3ListType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &GlmVec3ListType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &GlmVec3ListType::CallbackToString},
		{"__gc", &GlmVec3ListType::CallbackDelete},
		{"__len", &GlmVec3ListType::CallbackLength},
		{"__index", &GlmVec3ListType::CallbackDispatchGet},
		{"__newindex", &GlmVec3ListType::CallbackSet},
		{"insert", &GlmVec3ListType::CallbackInsert},
		{"remove", &GlmVec3ListType::CallbackRemove},
		{"resize", &GlmVec3ListType::CallbackResize},
		{nullptr, nullptr}
	};

	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");		// load global "meshproc"
	lua_newtable(lua());					// push new table on stack, which will become "meshproc.Mesh"
	luaL_setfuncs(lua(), staticFuncs, 0);	// Add static functions to new table
	lua_setfield(lua(), -2, "Vec3List");	// store new table as "Mesh" in "meshproc"; also pops that table
	lua_pop(lua(), 1);						// remove "meshproc" from stack

	return true;
}

void GlmVec3ListType::LuaPushElementValue(lua_State* lua, const std::vector<glm::vec3>& list, uint32_t indexZeroBased)
{
	GlmVec3Type::Push(lua, list.at(indexZeroBased));
}

bool GlmVec3ListType::LuaGetElement(lua_State* lua, int i, glm::vec3& outVal)
{
	return GlmVec3Type::TryGet(lua, i, outVal);
}

glm::vec3 GlmVec3ListType::GetInvalidValue()
{
	return glm::vec3{ 0.0f, 0.0f, 0.0f };
}
