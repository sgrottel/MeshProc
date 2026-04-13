#include "Cuboid.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>

using namespace meshproc;
using namespace meshproc::commands;

generator::Cuboid::Cuboid(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParamBinding<ParamMode::In, ParamType::Float>("SizeX", m_sizeX);
	AddParamBinding<ParamMode::In, ParamType::Float>("SizeY", m_sizeY);
	AddParamBinding<ParamMode::In, ParamType::Float>("SizeZ", m_sizeZ);
	AddParamBinding<ParamMode::In, ParamType::UInt32>("NumSegmentsX", m_segCntX);
	AddParamBinding<ParamMode::In, ParamType::UInt32>("NumSegmentsY", m_segCntY);
	AddParamBinding<ParamMode::In, ParamType::UInt32>("NumSegmentsZ", m_segCntZ);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("Mesh", m_mesh);
}

bool generator::Cuboid::Invoke()
{
	std::shared_ptr<data::Mesh> m = std::make_shared<data::Mesh>();
	const uint32_t cntX = (std::max)(1u, m_segCntX);
	const uint32_t cntY = (std::max)(1u, m_segCntY);
	const uint32_t cntZ = (std::max)(1u, m_segCntZ);

	// vertex indices based on loop values
	std::unordered_map<glm::uvec3, uint32_t> vs;

	auto addVertex = [&](uint32_t x, uint32_t y, uint32_t z)
		{
			const float ax = static_cast<float>(x) / static_cast<float>(cntX);
			const float ay = static_cast<float>(y) / static_cast<float>(cntY);
			const float az = static_cast<float>(z) / static_cast<float>(cntZ);
			vs.insert(std::make_pair(glm::uvec3{ x, y, z }, static_cast<uint32_t>(m->vertices.size())));
			m->vertices.push_back(glm::vec3{ ax * m_sizeX, ay * m_sizeY, az * m_sizeZ });
		};
	m->vertices.reserve(
		cntX * cntY * 2
		+ cntX * cntZ * 2
		+ cntY * cntZ * 2
		+ 2);

	// first add all vertices so that all quads can reference the final points
	for (uint32_t x = 0; x <= cntX; ++x)
	{
		for (uint32_t y = 0; y <= cntY; ++y)
		{
			for (uint32_t z = 0; z <= cntZ; z += cntZ)
			{
				addVertex(x, y, z);
			}
		}
	}
	for (uint32_t x = 0; x <= cntX; x += cntX)
	{
		for (uint32_t y = 0; y <= cntY; ++y)
		{
			for (uint32_t z = 1; z < cntZ; ++z)
			{
				addVertex(x, y, z);
			}
		}
	}
	for (uint32_t x = 1; x < cntX; ++x)
	{
		for (uint32_t y = 0; y <= cntY; y += cntY)
		{
			for (uint32_t z = 1; z < cntZ; ++z)
			{
				addVertex(x, y, z);
			}
		}
	}

	// now add quads
	for (uint32_t x = 0; x < cntX; ++x)
	{
		const bool xb = (x % 2) == 0;
		for (uint32_t y = 0; y < cntY; ++y)
		{
			const bool yb = (y % 2) == 0;

			m->AddQuad(
				vs.at(glm::uvec3{ x, y, cntZ }),
				vs.at(glm::uvec3{ x + 1, y, cntZ }),
				vs.at(glm::uvec3{ x, y + 1, cntZ }),
				vs.at(glm::uvec3{ x + 1, y + 1, cntZ }),
				xb == yb
			);
			m->AddQuad(
				vs.at(glm::uvec3{ x, y + 1, 0 }),
				vs.at(glm::uvec3{ x + 1, y + 1, 0 }),
				vs.at(glm::uvec3{ x, y, 0 }),
				vs.at(glm::uvec3{ x + 1, y, 0 }),
				xb == yb
			);
		}
	}
	for (uint32_t x = 0; x < cntX; ++x)
	{
		const bool xb = (x % 2) == 0;
		for (uint32_t z = 0; z < cntZ; ++z)
		{
			const bool zb = (z % 2) == 0;

			m->AddQuad(
				vs.at(glm::uvec3{ x, 0, z }),
				vs.at(glm::uvec3{ x + 1, 0, z }),
				vs.at(glm::uvec3{ x, 0, z + 1 }),
				vs.at(glm::uvec3{ x + 1, 0, z + 1 }),
				xb == zb
			);
			m->AddQuad(
				vs.at(glm::uvec3{ x, cntY, z + 1 }),
				vs.at(glm::uvec3{ x + 1, cntY, z + 1 }),
				vs.at(glm::uvec3{ x, cntY, z }),
				vs.at(glm::uvec3{ x + 1, cntY, z }),
				xb == zb
			);
		}
	}
	for (uint32_t y = 0; y < cntY; ++y)
	{
		const bool yb = (y % 2) == 0;
		for (uint32_t z = 0; z < cntZ; ++z)
		{
			const bool zb = (z % 2) == 0;

			m->AddQuad(
				vs.at(glm::uvec3{ 0, y, z + 1 }),
				vs.at(glm::uvec3{ 0, y + 1, z + 1 }),
				vs.at(glm::uvec3{ 0, y, z }),
				vs.at(glm::uvec3{ 0, y + 1, z }),
				yb == zb
			);
			m->AddQuad(
				vs.at(glm::uvec3{ cntX, y, z }),
				vs.at(glm::uvec3{ cntX, y + 1, z }),
				vs.at(glm::uvec3{ cntX, y, z + 1 }),
				vs.at(glm::uvec3{ cntX, y + 1, z + 1 }),
				yb == zb
			);
		}
	}

	m_mesh = m;
	return true;
}
