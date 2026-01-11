#include "VertexSurfaceDistanceToCut.h"

#include <SimpleLog/SimpleLog.hpp>

#include <numeric>
#include <unordered_set>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

using namespace meshproc;

VertexSurfaceDistanceToCut::VertexSurfaceDistanceToCut(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("PlaneOrigin", m_planeOrig);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("PlaneNormal", m_planeNorm);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("PlaneXAxis", m_planeX);
	AddParamBinding<ParamMode::In, ParamType::Float>("PlaneRectWidth", m_planeRectWidth); // along x
	AddParamBinding<ParamMode::In, ParamType::Float>("PlaneRectHeight", m_planeRectHeight);

	AddParamBinding<ParamMode::Out, ParamType::ListOfFloat>("Distances", m_dists);
}

bool VertexSurfaceDistanceToCut::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh not set");
		return false;
	}
	if (glm::length(m_planeNorm) < 0.00001f)
	{
		Log().Error("PlaneNormal too small (zero?)");
		return false;
	}
	if (m_planeRectWidth < 0.0f)
	{
		Log().Warning("Plane rect width smaller than zero is invalid");
	}
	if (m_planeRectHeight < 0.0f)
	{
		Log().Warning("Plane rect height smaller than zero is invalid");
	}

	const bool test2d = m_planeRectWidth > 0.0f && m_planeRectHeight > 0.0f;

	m_dists = std::make_shared<std::vector<float>>();
	m_dists->resize(m_mesh->vertices.size(), 0.0f);

	const glm::vec3 pNorm = glm::normalize(m_planeNorm);
	const glm::vec3 pY = glm::normalize(glm::cross(pNorm, m_planeX));
	if (glm::length(m_planeNorm) < 0.00001f)
	{
		Log().Error("PlaneXAxis and PlaneNormal seem similar");
		return false;
	}
	const glm::vec3 pX = glm::normalize(glm::cross(pY, pNorm));

	std::transform(
		m_mesh->vertices.begin(),
		m_mesh->vertices.end(),
		m_dists->begin(),
		[&](glm::vec3 const& v) {
			return glm::dot(v - m_planeOrig, pNorm);
		});

	std::unordered_set<size_t> triFront;
	std::unordered_set<size_t> tris;
	std::vector<bool> validVertDists;
	validVertDists.resize(m_dists->size());
	{
		std::unordered_map<data::HashableEdge, bool> validIn2D;
		float d[3];
		for (size_t i = 0; i < m_mesh->triangles.size(); ++i)
		{
			auto const& t = m_mesh->triangles.at(i);
			for (size_t j = 0; j < 3; ++j)
			{
				d[j] = m_dists->at(t[j]);
			}

			bool isCut = ((d[0] < 0.0f || d[1] < 0.0f || d[2] < 0.0f) && (d[0] >= 0.0f || d[1] >= 0.0f || d[2] >= 0.0f));

			if (isCut && test2d)
			{
				// potentially cut triangle
				isCut = false;
				for (size_t j = 0; j < 3; ++j)
				{
					const size_t j2 = (j + 1) % 3;
					if ((d[j] < 0.0f && d[j2] >= 0.0f) || (d[j] >= 0.0f && d[j2] < 0.0f))
					{
						data::HashableEdge e{ t[j], t[j2] };
						if (!validIn2D.contains(e))
						{
							const glm::vec3 v0 = m_mesh->vertices.at(t[j]) - m_planeOrig;
							const glm::vec3 v1 = m_mesh->vertices.at(t[j2]) - m_planeOrig;
							const glm::vec3 vt = (v0 * std::abs(d[j2]) + v1 * std::abs(d[j])) / (std::abs(d[j]) + std::abs(d[j2]));

#ifdef _DEBUG
							const float testDist = std::abs(glm::dot(vt, pNorm));
							assert(testDist < 0.0001f);
#endif

							const float x2 = glm::dot(vt, pX);
							const float y2 = glm::dot(vt, pY);

							const bool inRect = (std::abs(x2) <= m_planeRectWidth / 2.0f)
								&& (std::abs(y2) <= m_planeRectHeight / 2.0f);

							validIn2D.insert(std::make_pair(e, inRect));
						}
						if (validIn2D.at(e))
						{
							isCut = true;
							break;
						}
					}
				}
			}

			if (isCut)
			{
				triFront.insert(i);
				for (size_t j = 0; j < 3; ++j)
				{
					validVertDists.at(t[j]) = true;
				}
			}
			else
			{
				tris.insert(i);
			}
		}
	}

	while (!tris.empty())
	{
		triFront.clear();
		for (size_t tidx : tris)
		{
			auto const& t = m_mesh->triangles.at(tidx);
			const bool sel = validVertDists[t[0]] || validVertDists[t[1]] || validVertDists[t[2]];
			if (sel)
			{
				triFront.insert(tidx);
			}
		}
		if (triFront.empty())
		{
			tris.clear();
			continue;
		}
		for (size_t tidx : triFront)
		{
			tris.erase(tidx);
		}

		for (size_t tidx : triFront)
		{
			auto const& t = m_mesh->triangles.at(tidx);
			for (size_t j = 0; j < 3; ++j)
			{
				for (size_t k = 0; k < 3; ++k)
				{
					if (k == j) continue;
					if (!validVertDists.at(t[k])) continue;

					const float d = glm::distance(m_mesh->vertices.at(t[j]), m_mesh->vertices.at(t[k]));
					const float kDist = m_dists->at(t[k]);
					const float jDistAlt = (kDist < 0.0f) ? (kDist - d) : (kDist + d);

					if (!validVertDists.at(t[j]))
					{
						m_dists->at(t[j]) = jDistAlt;
						validVertDists.at(t[j]) = true;
					}
					else if (std::abs(m_dists->at(t[j])) > std::abs(jDistAlt))
					{
						m_dists->at(t[j]) = jDistAlt;
					}
				}
			}
		}

	}

	for (size_t i = 0; i < m_dists->size(); ++i)
	{
		if (!validVertDists.at(i))
		{
			m_dists->at(i) = (std::signbit(m_dists->at(i)) ? -1.0f : 1.0f) * std::numeric_limits<float>::max();
		}
	}

	return true;
}
