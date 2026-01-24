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
			class GlmVec3Type
			{
			public:
				static bool Push(lua_State* lua, const glm::vec3& val);
				static bool TryGet(lua_State* lua, int idx, glm::vec3& tar);

			private:
				GlmVec3Type() = delete;
			};
		}
	}
}
