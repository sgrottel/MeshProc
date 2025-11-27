#pragma once

#include "AbstractType.h"

#include <cstdint>
#include <vector>

namespace meshproc
{
	namespace lua
	{
		class ListOfFloatType : public AbstractType<std::vector<float>, ListOfFloatType>
		{
		public:
			static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.ListOfFloatType";

			ListOfFloatType(Runner& owner)
				: AbstractType<std::vector<float>, ListOfFloatType>{ owner }
			{};
			bool Init();

		protected:
			static int CallbackCtor(lua_State* lua);

			static int CallbackSize(lua_State* lua);
			static int CallbackGet(lua_State* lua);
			static int CallbackSet(lua_State* lua);
		};
	}
}

