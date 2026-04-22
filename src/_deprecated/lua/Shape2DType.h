#pragma once

#include "AbstractType.h"

namespace meshproc
{
	namespace data
	{
		class Shape2D;
	}

	namespace lua
	{
		class Shape2DType : public AbstractType<data::Shape2D, Shape2DType>
		{
		public:
			static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Shape2D";

			Shape2DType(Runner& owner)
				: AbstractType<data::Shape2D, Shape2DType>{ owner }
			{};
			bool Init();

		private:
			static int CallbackCtor(lua_State* lua);
			static int CallbackAdd(lua_State* lua);
			static int CallbackSize(lua_State* lua);
			static int CallbackId(lua_State* lua);
			static int CallbackGet(lua_State* lua);

		};
	}
}
