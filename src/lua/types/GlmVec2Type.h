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
			class GlmVec2Type
			{
			public:
				static bool Push(lua_State* lua, const glm::vec2& val);
				static bool TryGet(lua_State* lua, int idx, glm::vec2& tar);

			private:
				GlmVec2Type() = delete;
			};
		}
	}
}
