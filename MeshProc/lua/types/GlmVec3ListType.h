#pragma once

#include "AbstractListType.h"

#include <glm/glm.hpp>

#include <vector>

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			class GlmVec3ListType : public AbstractListType<glm::vec3, GlmVec3ListType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Vec3ListType";

				GlmVec3ListType(Runner& owner)
					: AbstractListType<glm::vec3, GlmVec3ListType>{ owner }
				{}
				bool Init();

			private:
				friend AbstractListType<glm::vec3, GlmVec3ListType>;

				static void LuaPushElementValue(lua_State* lua, const std::vector<glm::vec3>& list, uint32_t indexZeroBased);
				static bool LuaGetElement(lua_State* lua, int i, glm::vec3& outVal);
				static glm::vec3 GetInvalidValue();

			};
		}
	}
}
