#include "MeshType.h"

#include "data/Mesh.h"

#include "GlmUVec3Type.h"
#include "GlmVec3Type.h"

#include "lua/LuaUtilities.h"

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool MeshType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &MeshType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &MeshType::CallbackToString},
		{"__gc", &MeshType::CallbackDelete},

		{"vertexSize", &MeshType::CallbackVertexSize},
		{"vertexResize", &MeshType::CallbackVertexResize},
		{"vertexGet", &MeshType::CallbackVertexGet},
		{"vertexSet", &MeshType::CallbackVertexSet},

		{"triangleSize", &MeshType::CallbackTriangleSize},
		{"triangleResize", &MeshType::CallbackTriangleResize},
		{"triangleGet", &MeshType::CallbackTriangleGet},
		{"triangleSet", &MeshType::CallbackTriangleSet},

		{nullptr, nullptr}
	};

	if (!InitImpl(memberFuncs))
	{
		return false;
	}

	lua_getglobal(lua(), "meshproc");		// load global "meshproc"
	lua_newtable(lua());					// push new table on stack, which will become "meshproc.Mesh"
	luaL_setfuncs(lua(), staticFuncs, 0);	// Add static functions to new table
	lua_setfield(lua(), -2, "Mesh");		// store new table as "Mesh" in "meshproc"; also pops that table
	lua_pop(lua(), 1);						// remove "meshproc" from stack

	return true;
}

int MeshType::CallbackCtor(lua_State* lua)
{
	MeshType::LuaPush(lua, std::make_shared<data::Mesh>());
	return 1;
}

int MeshType::CallbackVertexSize(lua_State* lua)
{
	const int argcnt = lua_gettop(lua);
	if (argcnt != 1)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 1, is %d", argcnt);
	}
	const auto mesh = MeshType::LuaGet(lua, 1);
	if (!mesh)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Mesh");
	}

	lua_pushinteger(lua, mesh->vertices.size());
	return 1;
}

int MeshType::CallbackVertexResize(lua_State* lua)
{
	const int argcnt = lua_gettop(lua);
	if (argcnt != 2)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", argcnt);
	}
	const auto mesh = MeshType::LuaGet(lua, 1);
	if (!mesh)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Mesh");
	}

	uint32_t newSize;
	if (lua::GetLuaUint32(lua, 2, newSize) != GetResult::Ok)
	{
		return luaL_error(lua, "First argument expected to be an integer");
	}
	
	mesh->vertices.resize(newSize, glm::vec3{ 0, 0, 0 });

	return 0;
}

int MeshType::CallbackVertexGet(lua_State* lua)
{
	const int argcnt = lua_gettop(lua);
	if (argcnt != 2)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", argcnt);
	}
	const auto mesh = MeshType::LuaGet(lua, 1);
	if (!mesh)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Mesh");
	}

	uint32_t idx;
	if (lua::GetLuaUint32(lua, 2, idx) != GetResult::Ok)
	{
		return luaL_error(lua, "First argument expected to be an integer");
	}
	if (idx == 0 || idx > mesh->vertices.size())
	{
		return luaL_error(lua, "Argument out of bounds expected [1..%d], got %d", static_cast<int>(mesh->vertices.size()), static_cast<int>(idx));
	}

	GlmVec3Type::Push(lua, mesh->vertices.at(idx - 1));
	return 1;
}

int MeshType::CallbackVertexSet(lua_State* lua)
{
	// since Lua handles doubles/floats and integers the same, we can just use a xyz_math XVec3
	const int argcnt = lua_gettop(lua);
	if (argcnt != 3)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 3, is %d", argcnt);
	}
	const auto mesh = MeshType::LuaGet(lua, 1);
	if (!mesh)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Mesh");
	}

	uint32_t idx;
	if (lua::GetLuaUint32(lua, 2, idx) != GetResult::Ok)
	{
		return luaL_error(lua, "First argument expected to be an integer");
	}
	if (idx == 0 || idx > mesh->vertices.size())
	{
		return luaL_error(lua, "Argument out of bounds expected [1..%d], got %d", static_cast<int>(mesh->vertices.size()), static_cast<int>(idx));
	}

	if (!GlmVec3Type::TryGet(lua, 3, mesh->vertices.at(idx - 1)))
	{
		return luaL_error(lua, "Second argument expected to be a vec3");
	}

	return 0;
}

int MeshType::CallbackTriangleSize(lua_State* lua)
{
	const int argcnt = lua_gettop(lua);
	if (argcnt != 1)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 1, is %d", argcnt);
	}
	const auto mesh = MeshType::LuaGet(lua, 1);
	if (!mesh)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Mesh");
	}

	lua_pushinteger(lua, mesh->triangles.size());
	return 1;
}

int MeshType::CallbackTriangleResize(lua_State* lua)
{
	const int argcnt = lua_gettop(lua);
	if (argcnt != 2)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", argcnt);
	}
	const auto mesh = MeshType::LuaGet(lua, 1);
	if (!mesh)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Mesh");
	}

	uint32_t newSize;
	if (lua::GetLuaUint32(lua, 2, newSize) != GetResult::Ok)
	{
		return luaL_error(lua, "First argument expected to be an integer");
	}

	mesh->triangles.resize(newSize, data::Triangle{ 0, 0, 0 });

	return 0;
}

int MeshType::CallbackTriangleGet(lua_State* lua)
{
	// since Lua handles doubles/floats and integers the same, we can just use a xyz_math XVec3
	const int argcnt = lua_gettop(lua);
	if (argcnt != 2)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", argcnt);
	}
	const auto mesh = MeshType::LuaGet(lua, 1);
	if (!mesh)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Mesh");
	}

	uint32_t idx;
	if (lua::GetLuaUint32(lua, 2, idx) != GetResult::Ok)
	{
		return luaL_error(lua, "First argument expected to be an integer");
	}
	if (idx == 0 || idx > mesh->triangles.size())
	{
		return luaL_error(lua, "Argument out of bounds expected [1..%d], got %d", static_cast<int>(mesh->triangles.size()), static_cast<int>(idx));
	}

	const auto& t = mesh->triangles.at(idx - 1);
	GlmUVec3Type::Push(lua, glm::uvec3{ t[0], t[1], t[2] });
	return 1;
}

int MeshType::CallbackTriangleSet(lua_State* lua)
{
	// since Lua handles doubles/floats and integers the same, we can just use a xyz_math XVec3
	const int argcnt = lua_gettop(lua);
	if (argcnt != 3)
	{
		return luaL_error(lua, "Arguments number mismatch: must be 3, is %d", argcnt);
	}
	const auto mesh = MeshType::LuaGet(lua, 1);
	if (!mesh)
	{
		return luaL_error(lua, "Pre-First argument expected to be a Mesh");
	}

	uint32_t idx;
	if (lua::GetLuaUint32(lua, 2, idx) != GetResult::Ok)
	{
		return luaL_error(lua, "First argument expected to be an integer");
	}
	if (idx == 0 || idx > mesh->triangles.size())
	{
		return luaL_error(lua, "Argument out of bounds expected [1..%d], got %d", static_cast<int>(mesh->triangles.size()), static_cast<int>(idx));
	}

	glm::uvec3 tr;
	if (!GlmUVec3Type::TryGet(lua, 3, tr))
	{
		return luaL_error(lua, "Second argument expected to be a vec3");
	}

	auto& t = mesh->triangles.at(idx - 1);
	t[0] = tr.x;
	t[1] = tr.y;
	t[2] = tr.z;
	return 0;
}
