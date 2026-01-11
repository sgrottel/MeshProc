#pragma once

#include "AbstractType.h"

namespace meshproc
{
	namespace data
	{
		class Mesh;
	}

	namespace lua
	{
		namespace types
		{
			class MeshType : public AbstractType<data::Mesh, MeshType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Mesh";

				MeshType(Runner& owner)
					: AbstractType<data::Mesh, MeshType>{ owner }
				{};
				bool Init();

			private:
				static int CallbackCtor(lua_State* lua);

				static int CallbackVertexSize(lua_State* lua);
				static int CallbackVertexResize(lua_State* lua);
				static int CallbackVertexGet(lua_State* lua);
				static int CallbackVertexSet(lua_State* lua);

				static int CallbackTriangleSize(lua_State* lua);
				static int CallbackTriangleResize(lua_State* lua);
				static int CallbackTriangleGet(lua_State* lua);
				static int CallbackTriangleSet(lua_State* lua);

				static int CallbackApplyTransform(lua_State* lua);
				static int CallbackCalcBoundingBox(lua_State* lua);

			};
		}
	}
}
