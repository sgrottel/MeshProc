#pragma once

#include "AbstractListType.h"

#include <vector>

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			/*
			 * Note the index values stored in the vector are zero-based, because c++
			 * All getter and setter functions in the lua API add or remove 1 from the values to make the interface one-based
			 */
			class IndexListType : public AbstractListType<uint32_t, IndexListType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.IndexList";

				IndexListType(Runner& owner)
					: AbstractListType<uint32_t, IndexListType>{ owner }
				{}
				bool Init();

			private:
				friend AbstractListType<uint32_t, IndexListType>;

				static void LuaPushElementValue(lua_State* lua, const std::vector<uint32_t>& list, uint32_t indexZeroBased);
				static bool LuaGetElement(lua_State* lua, int i, uint32_t& outVal);
				static uint32_t GetInvalidValue();

			};
		}
	}
}
