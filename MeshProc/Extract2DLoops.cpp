#include "Extract2DLoops.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <array>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace meshproc;

Extract2DLoops::Extract2DLoops(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
	, m_mesh{ nullptr }
	, m_halfSpace{}
	, m_loops{ nullptr }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("PlaneNormal", m_halfSpace.GetPlaneNormalParam());
	AddParamBinding<ParamMode::In, ParamType::Float>("PlaneDist", m_halfSpace.GetPlaneDistParam());
	AddParamBinding<ParamMode::Out, ParamType::Shape2D>("Loops", m_loops);
}

bool Extract2DLoops::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh not set");
		return false;
	}
	if (!m_halfSpace.ValidateParams(Log()))
	{
		return false;
	}

	const glm::vec3 projXTemp = (std::abs(std::abs(m_halfSpace.Normal().x) - 1.0f) < 0.00001f)
		? glm::vec3{ 0.0f, 1.0f, 0.0f }
		: glm::vec3{ 1.0f, 0.0f, 0.0f };
	const glm::vec3 projY = glm::normalize(glm::cross(m_halfSpace.Normal(), projXTemp));
	const glm::vec3 projX = glm::normalize(glm::cross(projY, m_halfSpace.Normal()));

	std::unordered_map<glm::vec2, size_t> v2d;
	std::vector<std::array<size_t, 2>> edges2d;

	for (auto const& t : m_mesh->triangles)
	{
		glm::vec3 v[3];
		float dist[3];
		bool pos[3]; // is v in pos halfspace
		for (int i = 0; i < 3; ++i)
		{
			pos[i] = (dist[i] = m_halfSpace.Dist(v[i] = m_mesh->vertices[t[i]])) >= 0.0f;
		}

		if (pos[0] == pos[1] && pos[0] == pos[2])
		{
			// triangle entirely in one halfspace
			continue;
		}
		// else triangle crosses halfspace plane

		std::unordered_set<glm::vec2> pts;

		// TODO: this can be improved by hashing not over the point's coordinates, but over the hashable-edges of the triangles instead.
		auto mkPt = [](float x, float y)
			{
				constexpr double prec = 256 * 1024.0;
				x = static_cast<float>(std::round(x * prec) / prec);
				y = static_cast<float>(std::round(y * prec) / prec);
				return glm::vec2{ x, y };
			};

		for (int i = 0; i < 3; ++i) {
			const int j = (i + 1) % 3;
			if (pos[i] == pos[j]) continue;
			const float adi = abs(dist[i]);
			const float adj = abs(dist[j]);
			const float b = adi / (adi + adj);
			const float a = adj / (adi + adj);
			const glm::vec3 nv = v[i] * a + v[j] * b;

			pts.insert(mkPt(glm::dot(nv, projX), glm::dot(nv, projY)));
		}

		if (pts.size() < 2)
		{
			// degenerated to single point
			continue;
		}
		if (pts.size() > 2)
		{
			Log().Warning("Strange triangle with !=2 projected points: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)",
				v[0].x, v[0].y, v[0].z, v[1].x, v[1].y, v[1].z, v[2].x, v[2].y, v[2].z);
			continue;
		}

		// I got edge!
		auto addV = [&](const glm::vec2& p)
			{
				if (auto c = v2d.find(p); c != v2d.end())
				{
					return c->second;
				}
				const auto n = v2d.insert(std::make_pair(p, v2d.size()));
				return n.first->second;
			};
		std::array<size_t, 2> idx;
		std::transform(pts.begin(), pts.end(), idx.begin(), addV);
		if (idx[0] == idx[1])
		{
			Log().Warning("Edge degenerated");
			continue;
		}
		edges2d.push_back(idx);
	}

	m_loops = std::make_shared<data::Shape2D>();

	std::vector<glm::vec2> vec2d;
	vec2d.resize(v2d.size());
	for (auto const& p : v2d) {
		vec2d[p.second] = p.first;
	}

	// TODO: should be merged with the code in OpenBorder and moved to a utility function
	auto addLoop = [&](const std::vector<size_t>& l)
		{
			std::vector<glm::vec2> pl;
			pl.resize(l.size());
			std::transform(l.begin(), l.end(), pl.begin(), [&](size_t idx) { return vec2d.at(idx); });
			m_loops->loops.insert(std::make_pair(m_loops->loops.size(), std::move(pl)));
		};

	std::vector<std::vector<size_t>> polys;
	for (const std::array<size_t, 2>&e : edges2d)
	{
		auto i1 = std::find_if(polys.begin(), polys.end(), [i = e[0]](const std::vector<size_t>& l) { return l.front() == i || l.back() == i; });
		auto i2 = std::find_if(polys.begin(), polys.end(), [i = e[1]](const std::vector<size_t>& l) { return l.front() == i || l.back() == i; });

		if (i1 != polys.end())
		{
			if (i2 == i1)
			{
				// closing the loop
				addLoop(*i1);
				polys.erase(i1);
			}
			else
			if (i2 != polys.end())
			{
				// merge both lines
				if (i1->front() == e[0])
				{
					// need to connect i1 at the front (try if we can more easily connect to i2)

					i2->reserve(i2->size() + i1->size());
					if (i2->front() == e[1])
					{
						// need to connect i2 at the front
						// => reverse i2; then append i1 to i2
						std::reverse(i2->begin(), i2->end());
						for (auto ii = i1->begin(); ii != i1->end(); ++ii)
						{
							i2->push_back(*ii);
						}
					}
					else
					{
						// need to connect i2 at the end => append i1 to i2

						for (auto ii = i1->begin(); ii != i1->end(); ++ii)
						{
							i2->push_back(*ii);
						}
					}
					polys.erase(i1);
				}
				else
				{
					// need to connect i1 at the end
					i1->reserve(i2->size() + i1->size());
					if (i2->front() == e[1])
					{
						// need to connect i2 at the front => append i2 to i1

						for (auto ii = i2->begin(); ii != i2->end(); ++ii)
						{
							i1->push_back(*ii);
						}
					}
					else
					{
						// need to connect i2 at the end => append reversed i2 to i1
						for (auto ii = i2->rbegin(); ii != i2->rend(); ++ii)
						{
							i1->push_back(*ii);
						}
					}

					polys.erase(i2);
				}

			}
			else
			{
				if (i1->front() == e[0])
				{
					i1->insert(i1->begin(), e[1]);
				}
				else
				{
					i1->push_back(e[1]);
				}
			}
		}
		else
		{
			if (i2 != polys.end())
			{
				if (i2->front() == e[1])
				{
					i2->insert(i2->begin(), e[0]);
				}
				else
				{
					i2->push_back(e[0]);
				}
			}
			else
			{
				polys.push_back(std::vector<size_t>{e[0], e[1]});
			}
		}
	}

	if (polys.size() > 0)
	{
		Log().Warning("Open lines: %d", static_cast<int>(polys.size()));
		for (const auto& l : polys)
		{
			addLoop(l);
		}
	}

	Log().Detail("Extracted %d loops", static_cast<int>(m_loops->loops.size()));

	return true;
}
