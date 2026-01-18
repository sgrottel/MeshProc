#pragma once

#include "AbstractListType.h"
#include "AbstractType.h"

#include "data/Mesh.h"
#include "data/Triangle.h"

#include <glm/glm.hpp>

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			class MeshType : public AbstractType<data::Mesh, MeshType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Mesh";
				//static constexpr const char* LUA_SUBTYPE_TRIANGLELIST_NAME = "SGR.MeshProc.Data.Mesh_Triangle";

				static int LuaPush(lua_State* lua, std::shared_ptr<data::Mesh> val);

				MeshType(Runner& owner)
					: AbstractType<data::Mesh, MeshType>{ owner }
				{};
				bool Init();

			private:

				class VertexListTraits
				{
				public:
					using listptr_t = std::vector<glm::vec3>*;
					static listptr_t LuaGetList(lua_State* lua, int idx);
					static void OnInserted(lua_State* lua, int idx, listptr_t list, uint32_t idxZeroBase);
					static void OnRemoved(lua_State* lua, int idx, listptr_t list, uint32_t idxZeroBase);
					static void OnResized(lua_State* lua, int idx, listptr_t list, uint32_t newsize, uint32_t oldsize);
					static void OnManyRemoved(lua_State* lua, int idx, listptr_t list, std::vector<uint32_t>& idxListZeroBaseSortedAsc);
				};

				class Vertex : public AbstractListType<glm::vec3, Vertex, VertexListTraits>
				{
				public:
					static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Mesh_Vertex";

					using MyAbstractListType = AbstractListType<glm::vec3, Vertex, VertexListTraits>;

					using MyAbstractListType::CallbackToString;

					using MyAbstractListType::CallbackLength;
					using MyAbstractListType::CallbackDispatchGet;
					using MyAbstractListType::CallbackSet;
					using MyAbstractListType::CallbackInsert;
					using MyAbstractListType::CallbackRemove;
					using MyAbstractListType::CallbackResize;

					static int CallbackRemoveIsolated(lua_State* lua);

					static void LuaPushElementValue(lua_State* lua, const std::vector<glm::vec3>& list, uint32_t indexZeroBased);
					static bool LuaGetElement(lua_State* lua, int i, glm::vec3& outVal);
					static glm::vec3 GetInvalidValue();
				};

				class TriangleListTraits
				{
				public:
					using listptr_t = std::vector<data::Triangle>*;
					static listptr_t LuaGetList(lua_State* lua, int idx);
					static void OnInserted(lua_State* /*lua*/, int /*idx*/, listptr_t /*list*/, uint32_t /*idxZeroBase*/) {}
					static void OnRemoved(lua_State* /*lua*/, int /*idx*/, listptr_t /*list*/, uint32_t /*idxZeroBase*/) {}
					static void OnResized(lua_State* /*lua*/, int /*idx*/, listptr_t /*list*/, uint32_t /*newsize*/, uint32_t /*oldsize*/) {}
					static void OnManyRemoved(lua_State* /*lua*/, int /*idx*/, listptr_t /*list*/, std::vector<uint32_t>& /*idxListZeroBaseSortedAsc*/) {}
				};

				class Triangle : public AbstractListType<data::Triangle, Triangle, TriangleListTraits>
				{
				public:
					static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Mesh_Triangle";

					using MyAbstractListType = AbstractListType<data::Triangle, Triangle, TriangleListTraits>;

					using MyAbstractListType::CallbackToString;

					using MyAbstractListType::CallbackLength;
					using MyAbstractListType::CallbackDispatchGet;
					using MyAbstractListType::CallbackSet;
					using MyAbstractListType::CallbackInsert;
					using MyAbstractListType::CallbackRemove;
					using MyAbstractListType::CallbackResize;

					static void LuaPushElementValue(lua_State* lua, const std::vector<data::Triangle>& list, uint32_t indexZeroBased);
					static bool LuaGetElement(lua_State* lua, int i, data::Triangle& outVal);
					static data::Triangle GetInvalidValue();
				};

				static int CallbackCtor(lua_State* lua);

				static int CallbackIndexDispatch(lua_State* lua);

				static int CallbackApplyTransform(lua_State* lua);
				static int CallbackCalcBoundingBox(lua_State* lua);
				static int CallbackIsValid(lua_State* lua);

				int IsValid(lua_State* lua);

			};
		}
	}
}
