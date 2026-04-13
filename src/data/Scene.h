#pragma once

#include "data/Mesh.h"

#include <glm/glm.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace meshproc
{
	namespace data
	{

		class Scene
		{
		public:

			std::vector<std::pair<std::shared_ptr<Mesh>, glm::mat4>> m_meshes;

		};

	}
}