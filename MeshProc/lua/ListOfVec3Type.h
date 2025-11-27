#pragma once

#include "AbstractType.h"

#include <cstdint>
#include <glm/glm.hpp>

namespace meshproc
{
	namespace lua
	{
		class ListOfVec3Type : public AbstractType<std::vector<glm::vec3>, ListOfVec3Type>
		{
		public:
			static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.ListOfVec3";

			ListOfVec3Type(Runner& owner)
				: AbstractType<std::vector<glm::vec3>, ListOfVec3Type>{ owner }
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

