#pragma once

#include "AbstractType.h"

#include <cstdint>
#include <vector>

namespace meshproc
{
	namespace lua
	{
		class IndicesType : public AbstractType<std::vector<uint32_t>, IndicesType>
		{
		public:
			static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Indices";

			IndicesType(Runner& owner)
				: AbstractType<std::vector<uint32_t>, IndicesType>{ owner }
			{};
			bool Init();

		protected:
			static int CallbackSize(lua_State* lua);
			static int CallbackGet(lua_State* lua);
		};
	}
}
