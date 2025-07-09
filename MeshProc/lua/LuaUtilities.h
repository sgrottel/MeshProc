#pragma once

#include <cstdint>

// forward declaration
typedef struct lua_State lua_State;

namespace meshproc
{
	namespace lua
	{
		void DumpLuaStack(lua_State* lua);

		enum class GetResult
		{
			Ok,
			ErrorState,
			ErrorIndex,
			ErrorType,
			ErrorValue
		};

		GetResult GetLuaUint32(lua_State* lua, int i, uint32_t& tar);

	}
}
