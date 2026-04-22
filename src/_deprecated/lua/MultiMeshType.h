#pragma once

#include "AbstractMultiType.h"

namespace meshproc
{
	namespace data
	{
		class Mesh;
	}

	namespace lua
	{
		class MultiMeshType : public AbstractMultiType<data::Mesh, MultiMeshType>
		{
		public:
			static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.MultiMesh";

			MultiMeshType(Runner& owner)
				: AbstractMultiType<data::Mesh, MultiMeshType>{ owner }
			{};
			bool Init();
		protected:
			static int CallbackGet(lua_State* lua);
		};
	}
}
