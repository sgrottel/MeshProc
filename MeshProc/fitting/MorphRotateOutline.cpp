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
	template <typename T>
	typename std::enable_if_t<std::is_floating_point_v<T>, T>
	AngleDiff(T a, T b)
	{
		constexpr T pi = std::numbers::pi_v<T>;
		const T v = std::fmod(a - b, 2 * pi); // v is [-2pi..2pi]
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
		using ftype = double;
		using Vec3Type = glm::dvec3;
		using Vec2Type = glm::dvec2;
		using DiffT = glm::dvec2;

	private:
		static constexpr double pi = std::numbers::pi_v<ftype>;

	public:
		RollCoord() = default;
		RollCoord(const RollCoord& src) = default;
		RollCoord(RollCoord&& src) = default;
		RollCoord& operator=(const RollCoord& src) = default;
		RollCoord& operator=(RollCoord&& src) = default;

		explicit RollCoord(glm::vec3 const& v, CoordSys const& sys)
		{
			const Vec3Type r = v - sys.Origin();
			const ftype rl = glm::length(r);
			const Vec3Type c = glm::normalize(
				Vec3Type{
					glm::dot(static_cast<Vec3Type>(sys.X()), r),
					glm::dot(static_cast<Vec3Type>(sys.Y()), r),
					glm::dot(static_cast<Vec3Type>(sys.Z()), r)
				});
			const Vec2Type c2 = glm::normalize(Vec2Type{ c.x, c.y });

			m_roll = std::atan2(c2.y, c2.x);
			m_pitch = std::asin(c.z);
			m_dist = rl;

			const glm::vec3 recon = RollCoord::ToVec3(sys);
			const glm::vec3 diff = recon - v;
			const float diffVal = glm::length(diff);
			assert(diffVal < 0.0001f);
		}

		glm::vec3 ToVec3(CoordSys const& sys) const
		{
			const Vec3Type c = glm::normalize(
				Vec3Type{
					std::cos(m_roll) * std::cos(m_pitch),
					std::sin(m_roll) * std::cos(m_pitch),
					std::sin(m_pitch)
				}) * m_dist;

			return sys.Origin()
				+ sys.X() * static_cast<float>(c.x)
				+ sys.Y() * static_cast<float>(c.y)
				+ sys.Z() * static_cast<float>(c.z);
		}

		DiffT CalcRollTo(RollCoord const& rhs) const
		{
			return {
				AngleDiff(rhs.m_roll, m_roll),
				rhs.m_pitch - m_pitch
			};
		}

		ftype PrimaryAngle() const
		{
			return m_roll;
		}

		ftype Length() const
		{
			return m_dist;
		}

		void ApplyRoll(DiffT const & roll)
		{
			m_roll = AngleDiff(m_roll + roll.x, 0.0);
			m_pitch = std::clamp(m_pitch + roll.y, -pi, pi);
		}

	private:
		ftype m_roll;	// in XY plane ]-180°..180°]
		ftype m_pitch;	// pitch towards z [-90°..90°]
		ftype m_dist;
	};

	class SmoothRoll
	{
	private:
		static constexpr double pi = std::numbers::pi_v<double>;

	public:
		void Init(size_t num)
		{
			if (num < 2) throw std::logic_error("Sampling with too few points");
			m_val.resize(num);
		}

		void Build(std::vector<RollCoord::DiffT> const& roll, std::vector<RollCoord> const& from, double smooth = 1.0)
		{
			if (roll.size() != from.size()) throw std::logic_error("`roll` and `from` must be of same size");

			double smoothWidth = smooth * BucketWidth();
			auto CalcWeight = [smoothWidth](double angle)
				{
					assert(angle >= 0.0);
					if (angle > smoothWidth) return 0.0;
					return 1 - (angle / smoothWidth);
				};

			std::vector<size_t> emptyBuckets;

			for (size_t i = 0; i < m_val.size(); ++i)
			{
				const double samplingAngle = IndexToAngle(i);

				m_val.at(i) = RollCoord::DiffT{ 0, 0 };
				double w = 0;

				for (size_t j = 0; j < roll.size(); ++j)
				{
					const double jAngle = std::abs(AngleDiff(samplingAngle, static_cast<double>(from.at(j).PrimaryAngle())));
					const double jWeight = CalcWeight(jAngle);
					if (jWeight <= 0.0f) continue;

					m_val.at(i) += roll.at(j) * jWeight;
					w += jWeight;
				}

				if (w > 0)
				{
					m_val.at(i) /= w;
				}
				else
				{
					emptyBuckets.push_back(i);
				}
			}

			if (!emptyBuckets.empty())
			{
			}
		}

		RollCoord::DiffT Query(double angle) const
		{
			const size_t index1 = AngleToIndex(angle);
			const double angleDiff = AngleDiff(angle, IndexToAngle(index1));

			const size_t index2 = (index1 + ((angleDiff >= 0) ? 1 : (m_val.size() - 1))) % m_val.size();
			assert(index2 != index1);

			const double b = std::abs(angleDiff) / BucketWidth();
			assert(0 <= b && b <= 1);
			const double a = 1 - b;

			auto const& va = m_val.at(index1);
			auto const& vb = m_val.at(index2);

			return (va * a + vb * b);
		}

	private:
		std::vector<RollCoord::DiffT> m_val{}; // sampling points from [0°..360°[

		const double BucketWidth() const
		{
			return 2 * pi / static_cast<double>(m_val.size());
		}

		const double IndexToAngle(size_t idx) const
		{
			const double relDiff = static_cast<double>(idx) / static_cast<double>(m_val.size());
			const double angle = relDiff * 2 * pi;
			return angle;
		}

		const size_t AngleToIndex(double angle) const
		{
			assert(-pi <= angle && angle <= 2 * pi);
			const double a = std::fmod(angle + 2 * pi, 2 * pi); // [0..2pi[
			const double frac = a / (2 * pi); // [0..1[
			const double indexF = frac * m_val.size();
			const size_t index = static_cast<size_t>(indexF); // int clamp
			assert(0 <= index && index <= m_val.size());
			return index;
		}

	};

}

struct fitting::MorphRotateOutline::Impl
{
	flann::Matrix<float> targetDataSet{};
	flann::Index<flann::L2<float>> targetIndex{ flann::KDTreeIndexParams(1) };

	void BuildTargetIndex(const std::shared_ptr<data::Mesh> targetMesh)
	{
		targetDataSet = std::move(flann::Matrix<float>(
			glm::value_ptr(targetMesh->vertices.front()),
			targetMesh->vertices.size(),
			3));
		targetIndex = std::move(flann::Index<flann::L2<float>>(targetDataSet, flann::KDTreeIndexParams(1)));
		targetIndex.buildIndex();
	}

	CoordSys csys{ glm::vec3{ 0, 0, 0 }, glm::vec3{ 0, 0, 1 } };

	void BuildCoordinateSystem(const glm::vec3& center, const glm::vec3& primaryAxis)
	{
		const glm::vec3 axisRoll = glm::normalize(primaryAxis);
		{
			const float len = glm::length(axisRoll);
			if (len < 0.999 || len > 1.001)
			{
				throw std::runtime_error("PrimaryAxis normalization failed");
			}
		}
		csys = std::move(CoordSys{ center, axisRoll });
	}

	std::vector<uint32_t> border;
	size_t borderSize{ 0 };

	void SetBorder(std::vector<uint32_t>&& b)
	{
		border = std::move(b);
		borderSize = border.size();
	}

};

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

	m_impl = std::make_shared<Impl>();
	m_impl->BuildTargetIndex(m_targetMesh);
	m_impl->BuildCoordinateSystem(m_center, m_primaryAxis);
	m_impl->SetBorder(GetMeshBorderVertices());

	SmoothRotateIterate(90, 6, 0.2); // 0.2
	SmoothRotateIterate(90, 5, 0.25); // 0.2 + (0.8 * 0.25) = 0.4
	SmoothRotateIterate(90, 4, 0.333); // 0.4 + (0.6 * 0.33) ~= 0.6
	SmoothRotateIterate(90, 3, 0.5); // 0.6 + (0.4 * 0.5) = 0.8
	SmoothRotateIterate(90, 2, 1);

	m_impl.reset();
	return false;
}

void fitting::MorphRotateOutline::SmoothRotateIterate(size_t numBuckets, double smooth, double factor)
{
	assert(m_impl != nullptr);
	Log().Detail("SmoothRotateIterate(%d, %f, %f)", numBuckets, smooth, factor);

	std::vector<RollCoord> queryCoords(m_impl->borderSize);

	// prepare query
	std::vector<float> query_flat(m_impl->borderSize * 3);
	flann::Matrix<float> query(query_flat.data(), m_impl->borderSize, 3);
	for (size_t i = 0; i < m_impl->borderSize; ++i)
	{
		glm::vec3 const& v1 = m_mesh->vertices.at(m_impl->border.at(i));
		queryCoords.at(i) = RollCoord{ v1, m_impl->csys };

		glm::vec3 v2 = queryCoords.at(i).ToVec3(m_impl->csys);

		query_flat.at(i * 3 + 0) = v2.x;
		query_flat.at(i * 3 + 1) = v2.y;
		query_flat.at(i * 3 + 2) = v2.z;
	}

	// Allocate result buffers
	std::vector<int> indices_vec(m_impl->borderSize);
	std::vector<float> dists_vec(m_impl->borderSize);
	flann::Matrix<int> indices(indices_vec.data(), m_impl->borderSize, 1);
	flann::Matrix<float> dists(dists_vec.data(), m_impl->borderSize, 1);

	std::vector<RollCoord::DiffT> rollVal(m_impl->borderSize);

	m_impl->targetIndex.knnSearch(query, indices, dists, 1, flann::SearchParams(128));

	for (size_t i = 0; i < m_impl->borderSize; ++i)
	{
		RollCoord target{ m_targetMesh->vertices.at(indices_vec.at(i)), m_impl->csys };
		rollVal.at(i) = queryCoords.at(i).CalcRollTo(target);
	}

	SmoothRoll smoothRoll;
	smoothRoll.Init(numBuckets);
	smoothRoll.Build(rollVal, queryCoords, smooth);

	for (size_t i = 0; i < m_impl->borderSize; ++i)
	{
		auto roll = smoothRoll.Query(queryCoords.at(i).PrimaryAngle());
		queryCoords.at(i).ApplyRoll(roll * factor);
	}

	// TODO: Implement

	for (size_t i = 0; i < m_impl->borderSize; ++i)
	{
		m_mesh->vertices.at(m_impl->border.at(i)) = queryCoords.at(i).ToVec3(m_impl->csys);
		//m_mesh->vertices.at(border.at(i)) = m_targetMesh->vertices.at(indices_vec.at(i));
	}
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
