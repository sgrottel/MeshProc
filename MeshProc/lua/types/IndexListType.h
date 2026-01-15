#pragma once

#include "AbstractType.h"

#include <vector>

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			/*
			 * Note the index values stored in the vector are zero-based, because c++
			 * All getter and setter functions in the lua API add or remove 1 from the values to make the interface one-based
			 */
			class IndexListType : public AbstractType<std::vector<uint32_t>, IndexListType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.IndexList";

				IndexListType(Runner& owner)
					: AbstractType<std::vector<uint32_t>, IndexListType>{ owner }
				{};
				bool Init();

			private:
				static int CallbackCtor(lua_State* lua);

				static int CallbackLength(lua_State* lua);
				static int CallbackGet(lua_State* lua);
				static int CallbackSet(lua_State* lua);
				static int CallbackInsert(lua_State* lua);
				static int CallbackRemove(lua_State* lua);
				static int CallbackResize(lua_State* lua);
			};
		}
	}
}
