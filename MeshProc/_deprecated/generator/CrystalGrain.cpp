#include "CrystalGrain.h"

#include <SimpleLog/SimpleLog.hpp>

#include <cmath>
#include <unordered_set>
#include <random>

using namespace meshproc;

generator::CrystalGrain::CrystalGrain(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::UInt32>("RandomSeed", m_randomSeed);
	AddParamBinding<ParamMode::In, ParamType::UInt32>("SamplingShape", m_samplingShape); // TODO: Should be enum
	AddParamBinding<ParamMode::In, ParamType::UInt32>("NumFaces", m_numFaces);
	AddParamBinding<ParamMode::In, ParamType::Float>("SizeX", m_sizeX);
	AddParamBinding<ParamMode::In, ParamType::Float>("SizeY", m_sizeY);
	AddParamBinding<ParamMode::In, ParamType::Float>("SizeZ", m_sizeZ);
	AddParamBinding<ParamMode::In, ParamType::Float>("RadiusSigma", m_radSigma);
}

namespace
{
	struct Plane {
		// nx + ny + nz + d = 0
		glm::vec3 n;
		float d;
	};

	Plane AsPlane(glm::vec3 v)
	{
		float d = glm::length(v);
		return Plane{ glm::normalize(v), -d };
	}

	bool ThreePlanePoint(Plane const& a, Plane const& b, Plane const& c, glm::vec3& p)
	{
		glm::vec3 m1 = glm::cross(b.n, c.n);

		float denom = glm::dot(a.n, m1);
		if (std::abs(denom) < 0.000001f) return false;

		glm::vec3 m2 = glm::cross(c.n, a.n);
		glm::vec3 m3 = glm::cross(a.n, b.n);

		m1 *= -a.d;
		m2 *= -b.d;
		m3 *= -c.d;

		m1 += m2;
		m1 += m3;
		m1 /= denom;

		p = m1;

		return true;
	}

	class AbstractSampler {
	public:
		AbstractSampler(const glm::vec3 size, float radSigma)
			: m_rndRad{ 1.0f, std::max<float>(0.0001f, radSigma) }
			, m_size{ size }
		{ }
		virtual ~AbstractSampler() = default;
		virtual Plane operator()(std::default_random_engine& rnd) = 0;
	protected:
		std::normal_distribution<float> m_rndRad;
		const glm::vec3 m_size;
	};

	class SphereSampler final : public AbstractSampler {
	public:
		SphereSampler(const glm::vec3 size, float radSigma)
			: AbstractSampler{ size, radSigma }
			, m_rndNormal{}
		{
		}
		virtual ~SphereSampler() = default;
		Plane operator()(std::default_random_engine& rnd) override
		{
			glm::vec3 n{
					m_rndNormal(rnd),
					m_rndNormal(rnd),
					m_rndNormal(rnd)
			};
			n = glm::normalize(n);
			return AsPlane(n * m_size * m_rndRad(rnd));
		}
	private:
		std::normal_distribution<float> m_rndNormal;
	};

	class SquircleSampler final : public AbstractSampler {
	public:
		SquircleSampler(const glm::vec3 size, float radSigma)
			: AbstractSampler{ size, radSigma }
			, m_rndNormal{}
		{
		}
		virtual ~SquircleSampler() = default;
		Plane operator()(std::default_random_engine& rnd) override
		{
			glm::vec3 n{
					m_rndNormal(rnd),
					m_rndNormal(rnd),
					m_rndNormal(rnd)
			};
			n = glm::normalize(n * n * n * n * n);
			return AsPlane(n * m_size * m_rndRad(rnd));
		}
	private:
		std::normal_distribution<float> m_rndNormal;
	};

}

bool generator::CrystalGrain::Invoke()
{
	std::shared_ptr<data::Mesh> m = std::make_shared<data::Mesh>();

	std::vector<Plane> faces;

	const glm::vec3 size{ std::max<float>(0.1f, m_sizeX), std::max<float>(0.1f, m_sizeY), std::max<float>(0.1f, m_sizeZ) };
	std::unique_ptr<AbstractSampler> sampler;
	switch (m_samplingShape)
	{
	case 0:
		sampler = std::make_unique<SphereSampler>(size, m_radSigma);
		break;
	case 1:
		sampler = std::make_unique<SquircleSampler>(size, m_radSigma);
		break;
	default:
		sampler = std::make_unique<SphereSampler>(size, m_radSigma);
		break;
	}

	// guards ensuring the grain will never be too large
	faces.push_back(AsPlane(glm::vec3{ 2.0 * size.x, 0.0, 0.0 }));
	faces.push_back(AsPlane(glm::vec3{ -2.0 * size.x, 0.0, 0.0 }));
	faces.push_back(AsPlane(glm::vec3{ 0.0, 2.0 * size.y, 0.0 }));
	faces.push_back(AsPlane(glm::vec3{ 0.0, -2.0 * size.y, 0.0 }));
	faces.push_back(AsPlane(glm::vec3{ 0.0, 0.0, 2.0 * size.z }));
	faces.push_back(AsPlane(glm::vec3{ 0.0, 0.0, -2.0 * size.z }));

	std::default_random_engine rndEng{ m_randomSeed };
	for (uint32_t f = 0; f < m_numFaces; ++f)
	{
		faces.push_back((*sampler)(rndEng));
	}

	// filter planes with same normals (only keep smaller distance)
	{
		std::vector<Plane> fs;
		std::swap(fs, faces);
		faces.reserve(fs.size());
		for (const Plane& p : fs)
		{
			bool add = true;
			auto i = std::find_if(faces.begin(), faces.end(), [&p](Plane const& b) { return glm::distance(p.n, b.n) < 0.000001; });
			if (i != faces.end())
			{
				if (p.d > i->d)
				{
					faces.erase(i);
				}
				else
				{
					add = false;
				}
			}
			if (add)
			{
				faces.push_back(p);
			}
		}
	}

	std::vector<std::unordered_set<uint32_t>> loops;
	loops.resize(faces.size());

	for (size_t i = 0; i < faces.size(); ++i)
		for (size_t j = i + 1; j < faces.size(); ++j)
			for (size_t k = j + 1; k < faces.size(); ++k)
			{
				glm::vec3 p;
				if (!ThreePlanePoint(faces[i], faces[j], faces[k], p))
					continue;

				bool ok = true;
				for (size_t l = 0; l < faces.size(); ++l)
				{
					if (l == i || l == j || l == k) continue;
					float pd = glm::dot(p, faces[l].n) + faces[l].d;
					if (pd > 0.000001f)
					{
						ok = false;
						break;
					}
				}
				if (!ok) continue;

				size_t vidx = 0;
				for (; vidx < m->vertices.size(); ++vidx)
				{
					if (glm::distance(m->vertices[vidx], p) < 0.000001f)
					{
						break;
					}
				}
				if (vidx == m->vertices.size())
				{
					m->vertices.push_back(p);
				}

				loops[i].insert(static_cast<uint32_t>(vidx));
				loops[j].insert(static_cast<uint32_t>(vidx));
				loops[k].insert(static_cast<uint32_t>(vidx));
			}

	size_t tcnt = 0;
	for (auto const& l : loops) {
		if (loops.size() < 3) continue;
		tcnt += loops.size() - 2;
	}
	m->triangles.reserve(tcnt);

	std::vector<std::pair<float, uint32_t>> a;
	for (size_t i = 0; i < loops.size(); ++i) {
		auto const& l = loops[i];
		if (l.size() < 3) continue;
		a.clear();
		a.reserve(l.size());

		const glm::vec3 o = m->vertices[*(l.begin())];
		const glm::vec3 x = glm::normalize(m->vertices[*(++l.begin())] - o);
		const glm::vec3 y = glm::normalize(glm::cross(x, faces[i].n));

		for (uint32_t v : l)
		{
			const glm::vec3 v3 = m->vertices[v] - o;
			const float x2d = glm::dot(v3, x);
			const float y2d = glm::dot(v3, y);
			a.push_back({ std::atan2(y2d, x2d), v });
		}
		a.front().first = -10.0f;
		std::sort(a.begin(), a.end(), [](auto const& i, auto const& j) { return i.first < j.first; });

		for (size_t j = 2; j < a.size(); ++j)
		{
			m->triangles.push_back({ a[0].second, a[j].second, a[j - 1].second });
		}
	}

	m_mesh = m;
	return true;
}
