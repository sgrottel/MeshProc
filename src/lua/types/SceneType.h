#pragma once

#include "AbstractType.h"

namespace meshproc
{
	namespace data
	{
		class Scene;
	}

	namespace lua
	{
		namespace types
		{
			class SceneType : public AbstractType<data::Scene, SceneType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Scene";

				SceneType(Runner& owner)
					: AbstractType<data::Scene, SceneType>{ owner }
				{};
				bool Init();

			private:
				static int CallbackCtor(lua_State* lua);
				static int CallbackPlaceMesh(lua_State* lua);
				static int CallbackBake(lua_State* lua);

			};

		}
	}
}
