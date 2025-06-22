#pragma once

#include "AbstractType.h"

#include <cstdint>
#include <vector>

namespace meshproc
{
	namespace lua
	{
		class VertexSelectionType : public AbstractType<std::vector<uint32_t>, VertexSelectionType>
		{
		public:
			static constexpr const char* LUA_TYPE_NAME = "SGR.MeshProc.Data.VertexSelection";

			VertexSelectionType(Runner& owner)
				: AbstractType<std::vector<uint32_t>, VertexSelectionType>{ owner }
			{};
			bool Init();
		};
	}
}
