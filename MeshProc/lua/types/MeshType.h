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

				class Vertex : public AbstractListType<glm::vec3, Vertex>
				{
				public:
					static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Mesh_Vertex";

					using MyAbstractListType = AbstractListType<glm::vec3, Vertex>;

					using MyAbstractListType::CallbackToString;

					//using MyAbstractListType::CallbackLength;
					//using MyAbstractListType::CallbackDispatchGet;
					//using MyAbstractListType::CallbackSet;
					//using MyAbstractListType::CallbackInsert;
					//using MyAbstractListType::CallbackRemove;
					//using MyAbstractListType::CallbackResize;
				};

				class TriangleListTraits
				{
				public:
					using listptr_t = std::vector<data::Triangle>*;
					static listptr_t LuaGetList(lua_State* lua, int idx);
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

				static int CallbackVertexLength(lua_State* lua);
				static int CallbackVertexResize(lua_State* lua);
				static int CallbackVertexGet(lua_State* lua);
				static int CallbackVertexSet(lua_State* lua);
				static int CallbackVertexRemove(lua_State* lua);
				static int CallbackVertexRemoveIsolated(lua_State* lua);

				static int CallbackApplyTransform(lua_State* lua);
				static int CallbackCalcBoundingBox(lua_State* lua);
				static int CallbackIsValid(lua_State* lua);

			};
		}
	}
}
