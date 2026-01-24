#include "SceneType.h"

#include "lua/LuaUtilities.h"
#include "lua/types/GlmMat4Type.h"
#include "lua/types/MeshType.h"

#include "data/Scene.h"

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

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
		{"bake", &SceneType::CallbackBake},
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

int SceneType::CallbackBake(lua_State* lua)
{
	int size = lua_gettop(lua);
	if (size != 1)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 1, is %d", size);
	}
	auto scene = SceneType::LuaGet(lua, 1);
	if (!scene)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Scene");
	}

	std::shared_ptr<data::Mesh> all = std::make_shared<data::Mesh>();

	size_t vcnt = 0;
	size_t tcnt = 0;
	for (auto const& p : scene->m_meshes)
	{
		vcnt += p.first->vertices.size();
		tcnt += p.first->triangles.size();
	}

	all->vertices.reserve(vcnt);
	all->triangles.reserve(tcnt);

	vcnt = 0;
	for (auto const& p : scene->m_meshes)
	{
		for (auto const& v : p.first->vertices)
		{
			const glm::vec4 tv = p.second * glm::vec4{ v, 1.0f };
			all->vertices.push_back(glm::vec3{ tv } / tv.w);
		}
		for (auto const& t : p.first->triangles)
		{
			all->triangles.push_back({
				static_cast<uint32_t>(t[0] + vcnt),
				static_cast<uint32_t>(t[1] + vcnt),
				static_cast<uint32_t>(t[2] + vcnt)
				});
		}
		vcnt += p.first->vertices.size();
	}

	MeshType::LuaPush(lua, all);
	return 1;
}
