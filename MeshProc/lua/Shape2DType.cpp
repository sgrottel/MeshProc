#include "Shape2DType.h"

#include "GlmVec2Type.h"
#include "LuaUtilities.h"

#include "data/Shape2D.h"

using namespace meshproc;
using namespace meshproc::lua;

bool Shape2DType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &Shape2DType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &Shape2DType::CallbackToString},
		{"__gc", &Shape2DType::CallbackDelete},
		{"add", &Shape2DType::CallbackAdd},
		{nullptr, nullptr}
	};
	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");		// load global "meshproc"
	lua_newtable(lua());					// push new table on stack, which will become "meshproc.Scene"
	luaL_setfuncs(lua(), staticFuncs, 0);	// Add static functions to new table
	lua_setfield(lua(), -2, "Shape2D");		// store new table as "Scene" in "meshproc"; also pops that table
	lua_pop(lua(), 1);						// remove "meshproc" from stack

	return true;
}

int Shape2DType::CallbackCtor(lua_State* lua)
{
	Shape2DType::LuaPush(lua, std::make_shared<data::Shape2D>());
	return 1;
}

int Shape2DType::CallbackAdd(lua_State* lua)
{
	// DumpLuaStack(lua);

	int size = lua_gettop(lua);
	if (size != 2 && size != 3)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 3 or 2, is %d", size);
	}

	auto shape2d = Shape2DType::LuaGet(lua, 1);
	if (!shape2d)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Shape2D");
	}

	glm::vec2 pt;
	if (!GlmVec2Type::TryGet(lua, 2, pt))
	{
		return luaL_error(lua, "First argument expected to be a Mesh");
	}

	size_t loopIdx = 0;
	if (size == 3)
	{
		long long ll = luaL_checkinteger(lua, 3) - 1;
		if (ll < 0)
		{
			return luaL_error(lua, "Second argument loop index must be positive");
		}
		loopIdx = static_cast<size_t>(ll);
	}

	shape2d->loops[loopIdx].push_back(pt);

	return 0;
}
