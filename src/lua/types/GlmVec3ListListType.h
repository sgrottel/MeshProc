#pragma once

#include "AbstractListType.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			using GlmVec3ListListValueType = std::shared_ptr<std::vector<glm::vec3>>;

			class GlmVec3ListListType : public AbstractListType<GlmVec3ListListValueType, GlmVec3ListListType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Vec3ListList";

				GlmVec3ListListType(Runner& owner)
					: AbstractListType<GlmVec3ListListValueType, GlmVec3ListListType>{ owner }
				{}
				bool Init();

			private:
				friend AbstractListType<GlmVec3ListListValueType, GlmVec3ListListType>;

				static void LuaPushElementValue(lua_State* lua, const std::vector<GlmVec3ListListValueType>& list, uint32_t indexZeroBased);
				static bool LuaGetElement(lua_State* lua, int i, GlmVec3ListListValueType& outVal);
				static GlmVec3ListListValueType GetInvalidValue();

			};
		}
	}
}
