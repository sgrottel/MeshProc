#pragma once

#include "AbstractMultiType.h"

#include <cstdint>
#include <vector>

namespace meshproc
{
	namespace lua
	{
		class MultiIndicesType : public AbstractMultiType<std::vector<uint32_t>, MultiIndicesType>
		{
		public:
			static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.MultiIndices";

			MultiIndicesType(Runner& owner)
				: AbstractMultiType<std::vector<uint32_t>, MultiIndicesType>{ owner }
			{};
			bool Init();

		protected:
			static int CallbackGet(lua_State* lua);
		};
	}
}

