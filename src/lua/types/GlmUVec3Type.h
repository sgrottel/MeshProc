#pragma once

#include <glm/glm.hpp>

// forward declaration
typedef struct lua_State lua_State;

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			class GlmUVec3Type
			{
			public:
				static bool Push(lua_State* lua, const glm::uvec3& val);
				static bool TryGet(lua_State* lua, int idx, glm::uvec3& tar);

			private:
				GlmUVec3Type() = delete;
			};
		}
	}
}
