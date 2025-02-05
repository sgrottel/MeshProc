#pragma once

#include "Mesh.h"

#include <memory>

class CubeGenerator
{
public:
	static std::shared_ptr<Mesh> Create(float sizeX, float sizeY, float sizeZ);
};

