#include "MeshType.h"

#include "data/Mesh.h"

#include "GlmMat4Type.h"
#include "GlmUVec3Type.h"
#include "GlmVec3Type.h"

#include "lua/LuaUtilities.h"

#include <SimpleLog/SimpleLog.hpp>

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

		{"vertex_length", &MeshType::CallbackVertexLength},
		{"vertex_resize", &MeshType::CallbackVertexResize},
		{"vertex_get", &MeshType::CallbackVertexGet},
		{"vertex_set", &MeshType::CallbackVertexSet},
		{"vertex_remove", &MeshType::CallbackVertexRemove},
		{"vertex_remove_isolated", &MeshType::CallbackVertexRemoveIsolated},

		{"triangle_length", &MeshType::CallbackTriangleLength},
		{"triangle_resize", &MeshType::CallbackTriangleResize},
		{"triangle_get", &MeshType::CallbackTriangleGet},
		{"triangle_set", &MeshType::CallbackTriangleSet},
		{"triangle_remove", &MeshType::CallbackTriangleRemove},

		{"apply_transform", &MeshType::CallbackApplyTransform},
		{"calc_boundingbox", &MeshType::CallbackCalcBoundingBox},
		{"is_valid", &MeshType::CallbackIsValid},

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

int MeshType::CallbackVertexLength(lua_State* lua)
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

int MeshType::CallbackVertexRemove(lua_State* lua)
{
	return luaL_error(lua, "NOT IMPLEMENTED");
}

int MeshType::CallbackVertexRemoveIsolated(lua_State* lua)
{
	return luaL_error(lua, "NOT IMPLEMENTED");
}

int MeshType::CallbackTriangleLength(lua_State* lua)
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

	// triangle [0, 0, 0] is special as invalid -- keep
	// in all other cases increase vertex indices to be 1-based
	if (t[0] == 0 && t[1] == 0 && t[2] == 0)
	{
		GlmUVec3Type::Push(lua, glm::uvec3{ 0, 0, 0 });
		return 1;
	}

	GlmUVec3Type::Push(lua, glm::uvec3{ t[0] + 1, t[1] + 1, t[2] + 1 });
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

	// tr vertex indices are 1-based, unless all are zero
	if (tr.x != 0 && tr.y != 0 && tr.z != 0)
	{
		tr.x--;
		tr.y--;
		tr.z--;
	}
	else
	{
		// at least one is zero
		if (tr.x != 0 || tr.y != 0 || tr.z != 0)
		{
			// at least one is non-zero
			// => triangle does not use consistent 1-based indices
			return luaL_error(lua, "Vertex indices in triangle do not seem 1-based (%d, %d, &d)", tr.x, tr.y, tr.z);
		}
	}

	auto& t = mesh->triangles.at(idx - 1);
	t[0] = tr.x;
	t[1] = tr.y;
	t[2] = tr.z;
	return 0;
}

int MeshType::CallbackTriangleRemove(lua_State* lua)
{
	return luaL_error(lua, "NOT IMPLEMENTED");
}

int MeshType::CallbackApplyTransform(lua_State* lua)
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

	glm::mat4 mat{ 1.0f };
	if (!GlmMat4Type::TryGet(lua, 2, mat))
	{
		return luaL_error(lua, "Second argument expected to be a XMat4");
	}

	for (auto& v : mesh->vertices)
	{
		glm::vec4 hv = mat * glm::vec4{ v, 1 };
		v.x = hv.x / hv.w;
		v.y = hv.y / hv.w;
		v.z = hv.z / hv.w;
	}

	return 0;
}

int MeshType::CallbackCalcBoundingBox(lua_State* lua)
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

	if (mesh->vertices.size() <= 0)
	{
		return luaL_error(lua, "Mesh is empty");
	}

	glm::vec3 minBBox, maxBBox;
	minBBox = maxBBox = mesh->vertices.front();

	for (glm::vec3 const& v : mesh->vertices)
	{
		if (minBBox.x > v.x) minBBox.x = v.x;
		if (minBBox.y > v.y) minBBox.y = v.y;
		if (minBBox.z > v.z) minBBox.z = v.z;
		if (maxBBox.x < v.x) maxBBox.x = v.x;
		if (maxBBox.y < v.y) maxBBox.y = v.y;
		if (maxBBox.z < v.z) maxBBox.z = v.z;
	}

	GlmVec3Type::Push(lua, minBBox);
	GlmVec3Type::Push(lua, maxBBox);

	return 2;
}

int MeshType::CallbackIsValid(lua_State* lua)
{
	return luaL_error(lua, "NOT IMPLEMENTED");
}
