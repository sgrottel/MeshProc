#pragma once

#include "AbstractType.h"

namespace meshproc
{
	namespace data
	{
		class HalfSpace;
	}

	namespace lua
	{
		namespace types
		{
			class HalfSpaceType : public AbstractType<data::HalfSpace, HalfSpaceType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.HalfSpace";

				HalfSpaceType(Runner& owner)
					: AbstractType<data::HalfSpace, HalfSpaceType>{ owner }
				{};
				bool Init();

			private:
				static int CallbackCtor(lua_State* lua);
				static int CallbackSet(lua_State* lua);
				static int CallbackGet(lua_State* lua);
				static int CallbackDist(lua_State* lua);

			};

		}
	}
}
