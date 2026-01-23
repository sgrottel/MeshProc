#pragma once

#include "AbstractListType.h"

#include <vector>

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			class FloatListType : public AbstractListType<float, FloatListType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.FloatList";

				FloatListType(Runner& owner)
					: AbstractListType<float, FloatListType>{ owner }
				{}
				bool Init();

			private:
				friend AbstractListType<float, FloatListType>;

				static void LuaPushElementValue(lua_State* lua, const std::vector<float>& list, uint32_t indexZeroBased);
				static bool LuaGetElement(lua_State* lua, int i, float& outVal);
				static float GetInvalidValue();

			};
		}
	}
}
