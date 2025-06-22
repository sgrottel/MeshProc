#include "SceneType.h"

#include "GlmMat4Type.h"
#include "LuaUtilities.h"
#include "MeshType.h"

#include "data/Scene.h"

using namespace meshproc;
using namespace meshproc::lua;

bool SceneType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &SceneType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &SceneType::CallbackToString},
		{"__gc", &SceneType::CallbackDelete},
		{"place", &SceneType::CallbackPlaceMesh},
		{nullptr, nullptr}
	};
	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");		// load global "meshproc"
	lua_newtable(lua());					// push new table on stack, which will become "meshproc.Scene"
	luaL_setfuncs(lua(), staticFuncs, 0);	// Add static functions to new table
	lua_setfield(lua(), -2, "Scene");		// store new table as "Scene" in "meshproc"; also pops that table
	lua_pop(lua(), 1);						// remove "meshproc" from stack

	return true;
}

int SceneType::CallbackCtor(lua_State* lua)
{
	SceneType::LuaPush(lua, std::make_shared<data::Scene>());
	return 1;
}

int SceneType::CallbackPlaceMesh(lua_State* lua)
{
	// DumpLuaStack(lua);

	int size = lua_gettop(lua);
	if (size != 2 && size != 3)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 3 or 2, is %d", size);
	}

	auto scene = SceneType::LuaGet(lua, 1);
	if (!scene)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Scene");
	}

	auto mesh = MeshType::LuaGet(lua, 2);
	if (!mesh)
	{
		return luaL_error(lua, "First argument expected to be a Mesh");
	}

	glm::mat4 mat{ 1.0f };
	if (size == 3)
	{
		if (!GlmMat4Type::TryGet(lua, 3, mat))
		{
			return luaL_error(lua, "Second argument expected to be a XMat4");
		}
	}

	scene->m_meshes.push_back(std::make_pair(mesh, mat));

	return 0;
}
