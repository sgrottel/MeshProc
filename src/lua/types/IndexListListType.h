#pragma once

#include "AbstractListType.h"

#include <memory>
#include <vector>

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			using IndexListListValueType = std::shared_ptr<std::vector<uint32_t>>;

			class IndexListListType : public AbstractListType<IndexListListValueType, IndexListListType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.IndexListList";

				IndexListListType(Runner& owner)
					: AbstractListType<IndexListListValueType, IndexListListType>{ owner }
				{}
				bool Init();

			private:
				friend AbstractListType<IndexListListValueType, IndexListListType>;

				static void LuaPushElementValue(lua_State* lua, const std::vector<IndexListListValueType>& list, uint32_t indexZeroBased);
				static bool LuaGetElement(lua_State* lua, int i, IndexListListValueType& outVal);
				static IndexListListValueType GetInvalidValue();

			};
		}
	}
}
