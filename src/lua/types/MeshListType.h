#pragma once

#include "AbstractListType.h"

#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			using MeshListValueType = std::shared_ptr<data::Mesh>;

			class MeshListType : public AbstractListType<MeshListValueType, MeshListType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.MeshListType";

				MeshListType(Runner& owner)
					: AbstractListType<MeshListValueType, MeshListType>{ owner }
				{}
				bool Init();

			private:
				friend AbstractListType<MeshListValueType, MeshListType>;

				static void LuaPushElementValue(lua_State* lua, const std::vector<MeshListValueType>& list, uint32_t indexZeroBased);
				static bool LuaGetElement(lua_State* lua, int i, MeshListValueType& outVal);
				static MeshListValueType GetInvalidValue();

			};
		}
	}
}
