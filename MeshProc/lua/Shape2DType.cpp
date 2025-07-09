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
		{"size", &Shape2DType::CallbackSize}, // number of loops or size of loop by id
		{"id", &Shape2DType::CallbackId}, // id by number
		{"get", &Shape2DType::CallbackGet}, // point b from loop id = a
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

int Shape2DType::CallbackSize(lua_State* lua)
{
	const int size = lua_gettop(lua);
	if (size != 1 && size != 2)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 1 or 2, is %d", size);
	}
	const auto shape2d = Shape2DType::LuaGet(lua, 1);
	if (!shape2d)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Shape2D");
	}
	if (size == 1)
	{
		lua_pushinteger(lua, shape2d->loops.size());
	}
	else
	{
		assert(size == 2);
		uint32_t id;
		if (lua::GetLuaUint32(lua, 2, id) != GetResult::Ok)
		{
			return luaL_error(lua, "First argument expected to be an integer loop id");
		}

		if (!shape2d->loops.contains(id))
		{
			return luaL_error(lua, "First argument is an unknown loop id: %d", id);
		}

		lua_pushinteger(lua, shape2d->loops.at(id).size());
	}

	return 1;
}

int Shape2DType::CallbackId(lua_State* lua)
{
	const int size = lua_gettop(lua);
	if (size != 2)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", size);
	}
	const auto shape2d = Shape2DType::LuaGet(lua, 1);
	if (!shape2d)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Shape2D");
	}
	uint32_t idx;
	if (lua::GetLuaUint32(lua, 2, idx) != GetResult::Ok)
	{
		return luaL_error(lua, "First argument expected to be an integer index");
	}
	if (idx <= 0 || shape2d->loops.size() < idx)
	{
		return luaL_error(lua, "First argument out of bounds, expected [1..%d], got %d", shape2d->loops.size(), idx);
	}

	auto it = shape2d->loops.begin();
	std::advance(it, idx - 1);
	lua_pushinteger(lua, it->first);

	return 1;
}

int Shape2DType::CallbackGet(lua_State* lua)
{
	const int size = lua_gettop(lua);
	if (size != 3)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 3, is %d", size);
	}
	const auto shape2d = Shape2DType::LuaGet(lua, 1);
	if (!shape2d)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Shape2D");
	}
	uint32_t id;
	if (lua::GetLuaUint32(lua, 2, id) != GetResult::Ok)
	{
		return luaL_error(lua, "First argument expected to be an integer index");
	}
	if (!shape2d->loops.contains(id))
	{
		return luaL_error(lua, "First argument is an unknown loop id: %d", id);
	}
	auto const& loop = shape2d->loops.at(id);
	uint32_t idx;
	if (lua::GetLuaUint32(lua, 3, idx) != GetResult::Ok)
	{
		return luaL_error(lua, "Second argument expected to be an integer index");
	}
	if (idx <= 0 || loop.size() < idx)
	{
		return luaL_error(lua, "second argument out of bounds, expected [1..%d], got %d", loop.size(), idx);
	}

	GlmVec2Type::Push(lua, loop[idx - 1]);
	return 1;
}
