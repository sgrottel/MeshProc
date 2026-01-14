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

		// TODO: split into sub-object types for clearer access and use of "__len", "__index" (dispatcher), and "__newindex" along with the functions

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

/*

You’re thinking about this exactly the right way, and yes — if you do nothing special, then:

```lua
foo.bar:baz()
```

will call your C++ callback with **`self = bar`**, not `foo`.

But the good news is:
**Lua gives you a clean, idiomatic way to make `bar` act as a “view” or “sub?object” of `foo`, while still letting your C++ callback access the original `foo` without storing an explicit pointer in `bar`.**

Let’s walk through the architecture that makes this work.

---

# Key idea
Lua userdata can have **one uservalue table** (Lua 5.3) or **multiple uservalues** (Lua 5.4).
You can use this to attach the parent object (`foo`) to the child object (`bar`) *without storing it in your C++ struct*.

This is the cleanest and most idiomatic solution.

---

# The architecture you want

You want:

- `foo` ? a userdata representing your C++ object
- `foo.bar` ? a Lua table or userdata representing a *sub?object*
- `foo.bar` should still be able to call methods on `foo`
- `foo.bar:baz()` should call your C++ function with access to the original `foo`

This is exactly what uservalues are for.

---

# Step 1 — Create `foo` as userdata

```cpp
fooCpp* foo = (fooCpp*)lua_newuserdata(L, sizeof(fooCpp));
luaL_getmetatable(L, "FooMeta");
lua_setmetatable(L, -2);
```

---

# Step 2 — Create `foo.bar` as a Lua table (or userdata)

```cpp
lua_newtable(L);          // bar table
```

Now attach `foo` as the uservalue of `bar`:

```cpp
lua_pushvalue(L, -2);     // push foo
lua_setuservalue(L, -2);  // bar.uservalue = foo
```

Now:

- `bar` is a Lua object
- `bar` has a hidden reference to `foo`
- No explicit pointer stored in C++ memory

---

# Step 3 — Give `bar` its own metatable with methods

```cpp
luaL_getmetatable(L, "BarMeta");
lua_setmetatable(L, -2);
```

Inside `"BarMeta"` you define:

```cpp
static const luaL_Reg barMethods[] = {
	{"baz", &CallbackBaz},
	{NULL, NULL}
};
```

---

# Step 4 — In your callback, retrieve the parent `foo`

Your callback receives:

- `self = bar`

So you do:

```cpp
int CallbackBaz(lua_State* L) {
	// self = bar
	lua_getuservalue(L, 1);  // push bar.uservalue (which is foo)

	fooCpp* foo = (fooCpp*)lua_touserdata(L, -1);

	// now you have foo!
	...
}
```

This is the magic line:

```cpp
lua_getuservalue(L, 1);
```

It retrieves the parent object (`foo`) that you attached earlier.

---

# Result in Lua

```lua
foo.bar:baz()
```

- `self` is `bar`
- `bar.uservalue` is `foo`
- Your C++ callback can access both

---

# Summary

| Goal | Solution |
|------|----------|
| `foo.bar:baz()` should call method on `foo` | Store `foo` as uservalue of `bar` |
| Avoid storing explicit pointer in C++ struct | Use Lua uservalue table |
| Access parent from callback | `lua_getuservalue(L, 1)` |
| Keep clean metatable structure | Separate metatables for foo and bar |

---

# Final answer

**Yes, you can access `foo` from inside `baz` without storing a pointer in `bar`.
Attach `foo` as the uservalue of `bar`, and retrieve it in the callback with `lua_getuservalue`.**

This is the idiomatic Lua way to build hierarchical objects.

---

If you want, I can show you a complete minimal working example (C++ + Lua) that implements `foo.bar:baz()` exactly as you described.

*/

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
