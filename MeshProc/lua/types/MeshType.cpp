#include "MeshType.h"

#include "GlmMat4Type.h"
#include "GlmUVec3Type.h"
#include "GlmVec3Type.h"

#include "lua/LuaUtilities.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_set>

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

std::vector<glm::vec3>* MeshType::VertexListTraits::LuaGetList(lua_State* lua, int idx)
{
	luaL_checkudata(lua, idx, MeshType::Vertex::LUA_TYPE_NAME);
	lua_getuservalue(lua, idx);
	auto mesh = MeshType::LuaGet(lua, -1);
	lua_pop(lua, 1);
	return &mesh->vertices;
}

void MeshType::VertexListTraits::OnInserted(lua_State* lua, int idx, listptr_t list, uint32_t idxZeroBase)
{
	if (idxZeroBase + 1 == list->size())
	{
		// when a vertex was appended, and the mesh was valid before, aka all triangles referenced existing vertices,
		// then no triangle will reference the new vertex yet, or any vertex-idx beyond that.
		// So, no need to update the triangle data
		return;
	}

	luaL_checkudata(lua, idx, MeshType::Vertex::LUA_TYPE_NAME);
	lua_getuservalue(lua, idx);
	auto mesh = MeshType::LuaGet(lua, -1);
	lua_pop(lua, 1);

	for (data::Triangle& t : mesh->triangles)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			if (t[i] >= idxZeroBase)
			{
				t[i]++;
			}
		}
	}

}

void MeshType::VertexListTraits::OnRemoved(lua_State* lua, int idx, listptr_t list, uint32_t idxZeroBase)
{
	luaL_checkudata(lua, idx, MeshType::Vertex::LUA_TYPE_NAME);
	lua_getuservalue(lua, idx);
	auto mesh = MeshType::LuaGet(lua, -1);
	lua_pop(lua, 1);

	std::erase_if(mesh->triangles, [idxZeroBase](data::Triangle& t) { return t.HasIndex(idxZeroBase); });

	for (data::Triangle& t : mesh->triangles)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			if (t[i] > idxZeroBase)
			{
				t[i]--;
			}
		}
	}
}

void MeshType::VertexListTraits::OnResized(lua_State* lua, int idx, listptr_t list, uint32_t newsize, uint32_t oldsize)
{
	if (newsize >= oldsize) return;

	luaL_checkudata(lua, idx, MeshType::Vertex::LUA_TYPE_NAME);
	lua_getuservalue(lua, idx);
	auto mesh = MeshType::LuaGet(lua, -1);
	lua_pop(lua, 1);

	std::erase_if(mesh->triangles, [newsize](data::Triangle& t)
		{
			return t[0] >= newsize
				|| t[1] >= newsize
				|| t[2] >= newsize;
		});
}

void MeshType::VertexListTraits::OnManyRemoved(lua_State* lua, int idx, listptr_t /*list*/, std::vector<uint32_t>& idxListZeroBaseSortedAsc)
{
	if (idxListZeroBaseSortedAsc.empty()) return;

	luaL_checkudata(lua, idx, MeshType::Vertex::LUA_TYPE_NAME);
	lua_getuservalue(lua, idx);
	auto mesh = MeshType::LuaGet(lua, -1);
	lua_pop(lua, 1);

	std::unordered_map<uint32_t, uint32_t> remap;
	const uint32_t oldSize = static_cast<uint32_t>(mesh->vertices.size() + idxListZeroBaseSortedAsc.size());
	remap.reserve(mesh->vertices.size());
	uint32_t off = 0;
	for (uint32_t i = 0; i < oldSize; ++i)
	{
		if (off < idxListZeroBaseSortedAsc.size() && i == idxListZeroBaseSortedAsc.at(off))
		{
			off++;
		}
		else
		{
			remap.insert(std::make_pair(i, i - off));
		}
	}

	std::erase_if(mesh->triangles, [&remap](data::Triangle& t)
		{
			return !remap.contains(t[0])
				|| !remap.contains(t[1])
				|| !remap.contains(t[2]);
		});


	for (data::Triangle& t : mesh->triangles)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			t[i] = remap.at(t[i]);
		}
	}
}

int MeshType::Vertex::CallbackRemoveIsolated(lua_State* lua)
{
	luaL_checkudata(lua, -1, LUA_TYPE_NAME);
	lua_getuservalue(lua, -1);
	auto mesh = MeshType::LuaGet(lua, -1);
	lua_pop(lua, 1);
	mesh->RemoveIsolatedVertices();
	return 0;
}

void MeshType::Vertex::LuaPushElementValue(lua_State* lua, const std::vector<glm::vec3>& list, uint32_t indexZeroBased)
{
	GlmVec3Type::Push(lua, list.at(indexZeroBased));
}

bool MeshType::Vertex::LuaGetElement(lua_State* lua, int i, glm::vec3& outVal)
{
	return GlmVec3Type::TryGet(lua, i, outVal);
}

glm::vec3 MeshType::Vertex::GetInvalidValue()
{
	return { 0.0f, 0.0f, 0.0f };
}

std::vector<data::Triangle>* MeshType::TriangleListTraits::LuaGetList(lua_State* lua, int idx)
{
	luaL_checkudata(lua, idx, MeshType::Triangle::LUA_TYPE_NAME);
	lua_getuservalue(lua, idx);
	auto mesh = MeshType::LuaGet(lua, -1);
	lua_pop(lua, 1);
	return &(mesh->triangles);
}

void MeshType::Triangle::LuaPushElementValue(lua_State* lua, const std::vector<data::Triangle>& list, uint32_t indexZeroBased)
{
	const auto& t = list.at(indexZeroBased);

	// triangle [0, 0, 0] is special as invalid -- keep
	// in all other cases increase vertex indices to be 1-based
	if (t[0] == 0 && t[1] == 0 && t[2] == 0)
	{
		GlmUVec3Type::Push(lua, glm::uvec3{ 0, 0, 0 });
	}
	else
	{
		GlmUVec3Type::Push(lua, glm::uvec3{ t[0] + 1, t[1] + 1, t[2] + 1 });
	}
}

bool MeshType::Triangle::LuaGetElement(lua_State* lua, int i, data::Triangle& outVal)
{
	glm::uvec3 tr;
	if (!GlmUVec3Type::TryGet(lua, i, tr))
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

	outVal[0] = tr.x;
	outVal[1] = tr.y;
	outVal[2] = tr.z;

	return true;
}

data::Triangle MeshType::Triangle::GetInvalidValue()
{
	return data::Triangle{};
}

int MeshType::LuaPush(lua_State* lua, std::shared_ptr<data::Mesh> val)
{
	int retval = AbstractType<data::Mesh, MeshType>::LuaPush(lua, val);
	if (retval != 1)
	{
		// error case
		assert(retval == 0);
		return 0;
	}
	// top of stack is MeshType user object

	// Add a user value table to store fields
	lua_newtable(lua);
	lua_setuservalue(lua, -2);

	lua_getuservalue(lua, -1);

	lua_newuserdata(lua, 0);
	luaL_getmetatable(lua, MeshType::Vertex::LUA_TYPE_NAME);
	lua_setmetatable(lua, -2);
	lua_pushvalue(lua, -3);
	lua_setuservalue(lua, -2);
	lua_setfield(lua, -2, "vertex");

	lua_newuserdata(lua, 0);
	luaL_getmetatable(lua, MeshType::Triangle::LUA_TYPE_NAME);
	lua_setmetatable(lua, -2);
	lua_pushvalue(lua, -3);
	lua_setuservalue(lua, -2);

	lua_setfield(lua, -2, "triangle");

	lua_pop(lua, 1);

	return 1;
}

bool MeshType::Init()
{
	static const struct luaL_Reg staticFuncs[] = {
		{"new", &MeshType::CallbackCtor},
		{NULL, NULL}
	};

	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &MeshType::CallbackToString},
		{"__gc", &MeshType::CallbackDelete},
		{"__index", &MeshType::CallbackIndexDispatch},

		{"apply_transform", &MeshType::CallbackApplyTransform},
		{"calc_boundingbox", &MeshType::CallbackCalcBoundingBox},
		{"is_valid", &MeshType::CallbackIsValid},
		{"clone", &MeshType::CallbackClone},

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

	// Vertex user table!
	static const struct luaL_Reg vertexMemberFuncs[] = {
		{"__tostring", &Vertex::CallbackToString},
		{"__len", &Vertex::CallbackLength},
		{"__index", &Vertex::CallbackDispatchGet},
		{"__newindex", &Vertex::CallbackSet},
		{"insert", &Vertex::CallbackInsert},
		{"remove", &Vertex::CallbackRemove},
		{"resize", &Vertex::CallbackResize},

		{"remove_isolated", &Vertex::CallbackRemoveIsolated},

		{nullptr, nullptr}
	};

	if (!InitImpl(vertexMemberFuncs, MeshType::Vertex::LUA_TYPE_NAME))
	{
		return false;
	}

	// Triangle user table!
	static const struct luaL_Reg triangleMemberFuncs[] = {
		{"__tostring", &Triangle::CallbackToString},
		{"__len", &Triangle::CallbackLength},
		{"__index", &Triangle::CallbackDispatchGet},
		{"__newindex", &Triangle::CallbackSet},
		{"insert", &Triangle::CallbackInsert},
		{"remove", &Triangle::CallbackRemove},
		{"resize", &Triangle::CallbackResize},
		{nullptr, nullptr}
	};

	if (!InitImpl(triangleMemberFuncs, MeshType::Triangle::LUA_TYPE_NAME))
	{
		return false;
	}

	return true;
}

int MeshType::CallbackCtor(lua_State* lua)
{
	MeshType::LuaPush(lua, std::make_shared<data::Mesh>());
	return 1;
}

int MeshType::CallbackIndexDispatch(lua_State* lua)
{
	if (lua_getmetatable(lua, 1))
	{
		lua_pushvalue(lua, 2); // key
		lua_rawget(lua, -2); // metatable[key]
		if (!lua_isnil(lua, -1))
		{
			return 1; // found in metatable
		}
		lua_pop(lua, 1); // pop nil
		lua_pop(lua, 1); // pop metatable
	}
	
	lua_getuservalue(lua, 1);
	lua_pushvalue(lua, 2);
	lua_rawget(lua, -2);
	return 1;
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
	return CallLuaImpl(&MeshType::IsValid, lua);
}

int MeshType::IsValid(lua_State* lua)
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

	lua_pushboolean(lua, mesh->IsValid());
	return 1;
}

int MeshType::CallbackClone(lua_State* lua)
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

	std::shared_ptr<data::Mesh> clone = std::make_shared<data::Mesh>();
	clone->vertices.resize(mesh->vertices.size());
	std::copy(mesh->vertices.begin(), mesh->vertices.end(), clone->vertices.begin());
	clone->triangles.resize(mesh->triangles.size());
	std::copy(mesh->triangles.begin(), mesh->triangles.end(), clone->triangles.begin());

	LuaPush(lua, clone);
	return 1;
}
