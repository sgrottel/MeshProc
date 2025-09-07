#include "MorphRotateOutline.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/gtc/type_ptr.hpp>

#pragma warning(push)
#pragma warning(disable: 4267 4996)
#undef max
#undef min
#include <flann/flann.hpp>
#pragma warning(pop)

#include <cassert>
#include <numbers>
#include <stdexcept>
#include <unordered_set>

using namespace meshproc;

namespace
{
	class CoordSys
	{
	public:
		CoordSys(glm::vec3 const& o, glm::vec3 const& z)
			: m_origin{ o }
			, m_x { 0, 0, 0 }
			, m_y{ 0, 0, 0 }
			, m_z{ z }
		{
			assert(std::abs(glm::length(m_z) - 1.0f) < 0.0001f);
			constexpr const glm::vec3 gx{ 1.0, 0.0, 0.0 };

			if (std::abs(glm::dot(m_z, gx)) < 0.99)
			{
				// m_z and (1, 0, 0) are sufficiently different
				m_x = gx;
			}
			else
			{
				// m_z ~== gx
				m_x = glm::vec3{ 0.0f, 0.0f, 1.0f };
			}
			m_y = glm::normalize(glm::cross(m_z, m_x));
			m_x = glm::normalize(glm::cross(m_y, m_z));
		}

		glm::vec3 const& Origin() const noexcept
		{
			return m_origin;
		}
		glm::vec3 const& X() const noexcept
		{
			return m_x;
		}
		glm::vec3 const& Y() const noexcept
		{
			return m_y;
		}
		glm::vec3 const& Z() const noexcept
		{
			return m_z;
		}

	private:
		glm::vec3 m_origin;
		glm::vec3 m_x;
		glm::vec3 m_y;
		glm::vec3 m_z;
	};

	// a - b wraparound at 180°|pi
	// output range: ]-180°|-pi .. 180°|pi]
	double AngleDiff(double a, double b)
	{
		constexpr double pi = std::numbers::pi_v<double>;
		const double v = std::fmod(a - b, 2 * pi);
		if (v > pi)
		{
			return v - 2 * pi;
		}
		if (v <= -pi)
		{
			return v + 2 * pi;
		}
		return v;
	}

	class RollCoord
	{
	public:
		RollCoord() = default;
		RollCoord(const RollCoord& src) = default;
		RollCoord(RollCoord&& src) = default;
		RollCoord& operator=(const RollCoord& src) = default;
		RollCoord& operator=(RollCoord&& src) = default;

		explicit RollCoord(glm::vec3 const& v, CoordSys const& sys)
		{
			const glm::vec3 r = v - sys.Origin();
			const float rl = glm::length(r);
			const glm::vec3 c = glm::normalize(
				glm::vec3{ glm::dot(sys.X(), r),
				glm::dot(sys.Y(), r),
				glm::dot(sys.Z(), r) });
			const glm::vec2 c2 = glm::normalize(glm::vec2{ c.x, c.y });

			m_coord.x = std::atan2(c2.y, c2.x);
			m_coord.y = std::asin(c.z);
			m_coord.z = rl;
		}

		glm::vec3 ToVec3(CoordSys const& sys) const
		{
			const glm::dvec3 c{
				std::cos(m_coord.x) * std::cos(m_coord.y) * m_coord.z,
				std::sin(m_coord.x) * std::cos(m_coord.y) * m_coord.z,
				std::sin(m_coord.y) * m_coord.z };

			return sys.Origin()
				+ sys.X() * static_cast<float>(c.x)
				+ sys.Y() * static_cast<float>(c.y)
				+ sys.Z() * static_cast<float>(c.z);
		}

		glm::vec2 CalcRollTo(RollCoord const& rhs) const
		{
			return {
				AngleDiff(rhs.m_coord.x, m_coord.x),
				rhs.m_coord.y - m_coord.y
			};
		}

		float PrimaryAngle() const
		{
			return m_coord.x;
		}

		void ApplyRoll(glm::vec2 const & roll)
		{
			constexpr double pi = std::numbers::pi_v<double>;

			m_coord.x = AngleDiff(m_coord.x + roll.x, 0.0f);
			m_coord.y = std::clamp(m_coord.y + roll.y, -pi, pi);
		}

	private:
		glm::dvec3 m_coord{};
	};

	class SmoothRoll
	{
	public:
		void InitSampling(size_t num)
		{
			if (num < 2) throw std::logic_error("Sampling with too few points");
			m_val.resize(num);
		}

		void Sample(std::vector<glm::vec2> const& roll, std::vector<RollCoord> const& from, float smoothWidth = 0.0)
		{
			if (roll.size() != from.size()) throw std::logic_error("`roll` and `from` must be of same size");

			// assume "m_val.size() == 4"

			constexpr float pi = std::numbers::pi_v<float>;
			const float minSmoothWidth = (2 * pi) / m_val.size();
			// assume: minSmoothWidth = pi/2
			// assume: minSmoothWidth = pi (smooth!)

			if (smoothWidth <= minSmoothWidth)
			{
				smoothWidth = minSmoothWidth;
			}

			auto CalcWeight = [&smoothWidth](float angle)
				{
					// assume angle = pi/2
					constexpr float pi = std::numbers::pi_v<float>;
					const float a = angle / (smoothWidth / (pi / 2));
					// then a = pi/2 / ((pi/2) / pi/2) = pi/2 / 1 = pi/2
					// then a = pi/2 / (pi / pi/2) = pi/2 / 2 = pi/4
					if (a < -pi / 2 || a > pi / 2) return 0.0f;
					const float c = std::cos(a);
					// then c = 0
					// then c > 0
					return c * c;
				};

			for (size_t i = 0; i < m_val.size(); ++i)
			{
				const float samplingAngle = pi * static_cast<float>(2 * i) / static_cast<float>(m_val.size()); // angle sampling at

				m_val.at(i) = glm::vec2{ 0.0f, 0.0f };
				float w = 0.0f;

				for (size_t j = 0; j < roll.size(); ++j)
				{
					const float jAngle = AngleDiff(samplingAngle, from.at(j).PrimaryAngle());
					const float jWeight = 1; // CalcWeight(jAngle);
					if (jWeight <= 0.0f) continue;

					m_val.at(i) += roll.at(j) * jWeight;
					w += jWeight;
				}

				if (w > 0.0f)
				{
					m_val.at(i) /= w;
				}
			}
		}

		void Roll(RollCoord& c, float f)
		{
			constexpr float pi = std::numbers::pi_v<float>;
			float b = static_cast<float>(m_val.size()) * std::clamp(std::fmod(c.PrimaryAngle() + pi, 2 * pi), 0.0f, 2 * pi) / (2 * pi);
			size_t i = std::clamp<size_t>(static_cast<int>(b), 0, m_val.size() - 1);
			b = std::clamp(b - static_cast<float>(i), 0.0f, 1.0f);
			const float a = 1.0f - b;

			auto const& va = m_val.at(i);
			auto const& vb = m_val.at((i + 1) % m_val.size());

			c.ApplyRoll((va * a + vb * b) * f);
		}

	private:
		std::vector<glm::vec2> m_val{}; // sampling points from [0°..360°[
	};

}

fitting::MorphRotateOutline::MorphRotateOutline(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Mesh>("TargetMesh", m_targetMesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("Center", m_center);
	AddParamBinding<ParamMode::In, ParamType::Float>("FixRadius", m_fixRadius);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("PrimaryAxis", m_primaryAxis);
}

bool fitting::MorphRotateOutline::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (m_mesh->vertices.size() < 3)
	{
		Log().Error("Mesh must contain at least three vertices");
		return false;
	}
	if (!m_targetMesh)
	{
		Log().Error("TargetMesh is empty");
		return false;
	}
	if (m_targetMesh->vertices.size() < 3)
	{
		Log().Error("TargetMesh must contain at least three vertices");
		return false;
	}

	// assert that all vertices of the target mesh are stored continuous, aka can be used by flann without copy
	assert(static_cast<void const*>(m_targetMesh->vertices.data()) == static_cast<void const*>(glm::value_ptr(m_targetMesh->vertices.at(0))));
	assert(std::bit_cast<uint8_t const*>(glm::value_ptr(m_targetMesh->vertices.at(0)))
		+ 3 * sizeof(float)
		== static_cast<void const*>(glm::value_ptr(m_targetMesh->vertices.at(1))));
	assert(std::bit_cast<uint8_t const*>(glm::value_ptr(m_targetMesh->vertices.front()))
		+ (3 * sizeof(float)) * (m_targetMesh->vertices.size() - 1)
		== static_cast<void const*>(glm::value_ptr(m_targetMesh->vertices.back())));

	flann::Matrix<float> targetDataSet(
		glm::value_ptr(m_targetMesh->vertices.front()),
		m_targetMesh->vertices.size(),
		3);
	flann::Index<flann::L2<float>> targetIndex(targetDataSet, flann::KDTreeIndexParams(1));
	targetIndex.buildIndex();

	const glm::vec3 axisRoll = glm::normalize(m_primaryAxis);
	{
		const float len = glm::length(axisRoll);
		if (len < 0.999 || len > 1.001)
		{
			Log().Error("PrimaryAxis normalization failed");
			return false;
		}
	}
	const CoordSys csys{ m_center, axisRoll };

	std::vector<uint32_t> border = GetMeshBorderVertices();
	const size_t borderSize = border.size();

	std::vector<RollCoord> queryCoords(borderSize);

	// prepare query
	std::vector<float> query_flat(borderSize * 3);
	flann::Matrix<float> query(query_flat.data(), borderSize, 3);
	for (size_t i = 0; i < borderSize; ++i)
	{
		glm::vec3 const& v1 = m_mesh->vertices.at(border.at(i));
		queryCoords.at(i) = RollCoord{ v1, csys };

		glm::vec3 v2 = queryCoords.at(i).ToVec3(csys);

		query_flat.at(i * 3 + 0) = v2.x;
		query_flat.at(i * 3 + 1) = v2.y;
		query_flat.at(i * 3 + 2) = v2.z;
	}

	// Allocate result buffers
	std::vector<int> indices_vec(borderSize);
	std::vector<float> dists_vec(borderSize);
	flann::Matrix<int> indices(indices_vec.data(), borderSize, 1);
	flann::Matrix<float> dists(dists_vec.data(), borderSize, 1);

	std::vector<glm::vec2> rollVal(borderSize);

	targetIndex.knnSearch(query, indices, dists, 1, flann::SearchParams(128));

	for (size_t i = 0; i < borderSize; ++i)
	{
		RollCoord target{ m_targetMesh->vertices.at(indices_vec.at(i)), csys };
		rollVal.at(i) = queryCoords.at(i).CalcRollTo(target);
	}
	
	SmoothRoll smoothRoll;
	smoothRoll.InitSampling(72); // ok
	smoothRoll.Sample(rollVal, queryCoords, 0 * std::numbers::pi_v<float> / 32);

	for (size_t i = 0; i < borderSize; ++i)
	{
		//smoothRoll.Roll(queryCoords.at(i), 1);
		queryCoords.at(i).ApplyRoll(rollVal.at(i));
	}

	// TODO: Implement

	for (size_t i = 0; i < borderSize; ++i)
	{
		// TODO: this is quite imprecise. Why?
		m_mesh->vertices.at(border.at(i)) = queryCoords.at(i).ToVec3(csys);

		//m_mesh->vertices.at(border.at(i)) = m_targetMesh->vertices.at(indices_vec.at(i));
	}

	return false;
}

std::vector<uint32_t> fitting::MorphRotateOutline::GetMeshBorderVertices() const
{
	std::unordered_set<data::HashableEdge> border;
	for (auto const& t : m_mesh->triangles)
	{
		for (uint32_t i = 0; i < 3; ++i)
		{
			auto edge = t.HashableEdge(i);
			if (!border.insert(edge).second)
			{
				border.erase(edge);
			}
		}
	}

	std::unordered_set<uint32_t> vertices;
	for (auto const& e : border)
	{
		vertices.insert(e.i0);
		vertices.insert(e.i1);
	}

	std::vector<uint32_t> result;
	result.resize(vertices.size());

	std::copy(vertices.begin(), vertices.end(), result.begin());

	return result;
}
