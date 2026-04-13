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
			class GlmMat4Type
			{
			public:
				static bool Push(lua_State* lua, const glm::mat4& val);
				static bool TryGet(lua_State* lua, int idx, glm::mat4& tar);

			private:
				GlmMat4Type() = delete;
			};
		}
	}
}
