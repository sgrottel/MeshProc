#pragma once

#include "AbstractType.h"

namespace meshproc
{
	namespace data
	{
		class Mesh;
	}

	namespace lua
	{
		namespace types
		{
			class MeshType : public AbstractType<data::Mesh, MeshType>
			{
			public:
				static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.Mesh";

				MeshType(Runner& owner)
					: AbstractType<data::Mesh, MeshType>{ owner }
				{};
				bool Init();
			};
		}
	}
}
