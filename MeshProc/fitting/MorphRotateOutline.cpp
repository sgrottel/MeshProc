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

	// MAX exclusive
	template <typename T, T MIN, T NEUTRAL, T MAX>
	class CyclicFloat
	{
		static_assert(std::is_floating_point<T>::value, "T must be a floating-point type");
		static_assert(MIN <= NEUTRAL, "MIN must be less or equal NEUTRAL");
		static_assert(NEUTRAL <= MAX, "NEUTRAL must be less or equal MAX");
		static_assert(MIN < MAX, "MIN must be less MAX");

		constexpr static const T LEN = MAX - MIN;

	public:
		CyclicFloat() : m_value{ NEUTRAL } {}
		CyclicFloat(T value)
			: m_value{ NEUTRAL }
		{
			(*this) = value;
		}
		~CyclicFloat() = default;
		CyclicFloat(CyclicFloat const& src) : m_value{ src.m_value } {}
		CyclicFloat(CyclicFloat&& src) : m_value{ src.m_value } {}
		CyclicFloat& operator=(CyclicFloat const& src)
		{
			m_value = src.m_value;
			return *this;
		}
		CyclicFloat& operator=(CyclicFloat&& src)
		{
			m_value = src.m_value;
			return *this;
		}

		CyclicFloat& operator=(T value)
		{
			if (MIN <= value && value <= MAX)
			{
				m_value = value;
			}
			else
			{
				m_value = MIN + std::fmod(value - MIN, LEN);
			}
			return *this;
		}

		inline T GetValue() const noexcept
		{
			return m_value;
		}

		//  a - b
		T Minus(CyclicFloat const& b) const
		{
			const T d = m_value - b;
			if (m_value >= b)
			{
				assert(d >= 0 && d <= LEN);
				if (d > LEN / 2)
				{
					return d - LEN;
				}
			}
			else
			{
				assert(d <= 0 && d >= -LEN);
				if (d < LEN / 2)
				{
					return d + LEN;
				}
			}
			return d;
		}

		CyclicFloat& operator+(T b)
		{
			return (*this) = (m_value + b);
		}

	private:
		T m_value;
	};

	template <typename T, T MIN, T NEUTRAL, T MAX>
	class ClampedFloat
	{
		static_assert(std::is_floating_point<T>::value, "T must be a floating-point type");
		static_assert(MIN <= NEUTRAL, "MIN must be less or equal NEUTRAL");
		static_assert(NEUTRAL <= MAX, "NEUTRAL must be less or equal MAX");
		static_assert(MIN < MAX, "MIN must be less MAX");

		constexpr static const T LEN = MAX - MIN;

	public:
		ClampedFloat() : m_value{ NEUTRAL } {}
		ClampedFloat(T value)
			: m_value{ NEUTRAL }
		{
			(*this) = value;
		}
		~ClampedFloat() = default;
		ClampedFloat(ClampedFloat const& src) : m_value{ src.m_value } {}
		ClampedFloat(ClampedFloat&& src) : m_value{ src.m_value } {}
		ClampedFloat& operator=(ClampedFloat const& src)
		{
			m_value = src.m_value;
			return *this;
		}
		ClampedFloat& operator=(ClampedFloat&& src)
		{
			m_value = src.m_value;
			return *this;
		}

		ClampedFloat& operator=(T value)
		{
			if (value < MIN)
			{
				m_value = MIN;
			}
			else if (value > MAX)
			{
				m_value = MAX;
			}
			else
			{
				m_value = value;
			}
			return *this;
		}

		inline operator T() const noexcept
		{
			return m_value;
		}

	private:
		T m_value;
	};

	struct MapCoord
	{
		static constexpr double pi = std::numbers::pi_v<double>;

		CyclicFloat<double, -pi, 0.0, pi> m_primeAng;
		ClampedFloat<double, -pi / 2, 0.0, pi / 2> m_secAng;
		double m_length;
		CyclicFloat<double, 0.0, 0.0, 1.0> m_param;
	};

	class CoordSys
	{
	public:
		CoordSys(glm::vec3 const& o, glm::vec3 const& z, std::vector<glm::vec3> const& border)
			: m_origin{ o }
			, m_x { 0, 0, 0 }
			, m_y{ 0, 0, 0 }
			, m_z{ z }
		{
			assert(std::abs(glm::length(m_z) - 1.0) < 0.0001);
			constexpr const glm::dvec3 gx{ 1.0, 0.0, 0.0 };

			if (std::abs(glm::dot(m_z, gx)) < 0.99)
			{
				// m_z and (1, 0, 0) are sufficiently different
				m_x = gx;
			}
			else
			{
				// m_z ~== gx
				m_x = glm::dvec3{ 0.0, 0.0, 1.0 };
			}
			m_y = glm::normalize(glm::cross(m_z, m_x));
			m_x = glm::normalize(glm::cross(m_y, m_z));

			assert(border.size() > 2);
			m_border.resize(border.size());
			std::copy(border.begin(), border.end(), m_border.begin());

			m_borderDataSet = std::move(flann::Matrix<double>(
				glm::value_ptr(m_border.front()),
				m_border.size(),
				3));
			m_borderIndex = std::move(flann::Index<flann::L2<double>>(m_borderDataSet, flann::KDTreeIndexParams(1)));
			m_borderIndex.buildIndex();

		}

		MapCoord Map(glm::vec3 const& v) const
		{
			const glm::dvec3 r = glm::dvec3{ v } - m_origin;

			const double rl = glm::length(r);

			const glm::dvec3 c = glm::normalize(
				glm::dvec3{
					glm::dot(m_x, r),
					glm::dot(m_y, r),
					glm::dot(m_z, r) });
			const glm::dvec2 c2 = glm::normalize(glm::dvec2{ c.x, c.y });

			const double roll = std::atan2(c2.y, c2.x);
			const double pitch = std::asin(c.z);

			// TODO: Impl param space value

			const MapCoord mc{
				.m_primeAng = roll,
				.m_secAng = pitch,
				.m_length = rl,
				.m_param = 0.0
			};
#if _DEBUG
			const glm::vec3 v2 = Unmap(mc);
			const float diffVal = glm::distance(v, v2);
			assert(diffVal >= 0 && diffVal < 0.0001f);
#endif
			return mc;
		}

		glm::vec3 Unmap(MapCoord const& m) const
		{
			const glm::dvec3 c = glm::normalize(
				glm::dvec3{
					std::cos(m.m_primeAng.GetValue()) * std::cos(m.m_secAng),
					std::sin(m.m_primeAng.GetValue()) * std::cos(m.m_secAng),
					std::sin(m.m_secAng)
				}) * m.m_length;

			const glm::dvec3 v = m_origin
				+ m_x * c.x
				+ m_y * c.y
				+ m_z * c.z;
			return glm::vec3{ v };
		}

		//glm::vec3 const& Origin() const noexcept
		//{
		//	return m_origin;
		//}
		//glm::vec3 const& X() const noexcept
		//{
		//	return m_x;
		//}
		//glm::vec3 const& Y() const noexcept
		//{
		//	return m_y;
		//}
		//glm::vec3 const& Z() const noexcept
		//{
		//	return m_z;
		//}

	private:
		glm::dvec3 m_origin;
		glm::dvec3 m_x;
		glm::dvec3 m_y;
		glm::dvec3 m_z;
		std::vector<glm::dvec3> m_border;
		flann::Matrix<double> m_borderDataSet{};
		flann::Index<flann::L2<double>> m_borderIndex{ flann::KDTreeIndexParams(1) };
	};

	/*
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
	*/
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

	CoordSys csys{ glm::vec3{ 0, 0, 0 }, glm::vec3{ 0, 0, 1 }, { glm::vec3{}, glm::vec3{}, glm::vec3{} } };

	void BuildCoordinateSystem(const glm::vec3& center, const glm::vec3& primaryAxis, const std::shared_ptr<data::Mesh> mesh, const std::vector<uint32_t>& border)
	{
		(border); // todo compute better unfolding space
		const glm::vec3 axisRoll = glm::normalize(primaryAxis);
		{
			const float len = glm::length(axisRoll);
			if (len < 0.999 || len > 1.001)
			{
				throw std::runtime_error("PrimaryAxis normalization failed");
			}
		}
		
		std::vector<glm::vec3> b{ border.size() };
		std::transform(border.begin(), border.end(), b.begin(), [&mesh](uint32_t i) { return mesh->vertices.at(i); });

		csys = std::move(CoordSys{ center, axisRoll, b });
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
	m_impl->SetBorder(GetMeshBorderVertices());
	m_impl->BuildCoordinateSystem(m_center, m_primaryAxis, m_mesh, m_impl->border);

	//SmoothRotateIterate(90, 6, 0.2); // 0.2
	//SmoothRotateIterate(90, 5, 0.25); // 0.2 + (0.8 * 0.25) = 0.4
	//SmoothRotateIterate(90, 4, 0.333); // 0.4 + (0.6 * 0.33) ~= 0.6
	//SmoothRotateIterate(90, 3, 0.5); // 0.6 + (0.4 * 0.5) = 0.8
	//SmoothRotateIterate(90, 2, 1);

	SmoothRotateIterate(90, 5, 1);

	m_impl.reset();
	return false;
}

void fitting::MorphRotateOutline::SmoothRotateIterate(size_t numBuckets, double smooth, double factor)
{
	assert(m_impl != nullptr);
	Log().Detail("SmoothRotateIterate(%d, %f, %f)", numBuckets, smooth, factor);

	std::vector<MapCoord> queryCoords(m_impl->borderSize);

	// prepare query
	std::vector<float> query_flat(m_impl->borderSize * 3);
	flann::Matrix<float> query(query_flat.data(), m_impl->borderSize, 3);
	for (size_t i = 0; i < m_impl->borderSize; ++i)
	{
		glm::vec3 const& v1 = m_mesh->vertices.at(m_impl->border.at(i));
		queryCoords.at(i) = m_impl->csys.Map(v1);

		query_flat.at(i * 3 + 0) = v1.x;
		query_flat.at(i * 3 + 1) = v1.y;
		query_flat.at(i * 3 + 2) = v1.z;
	}

	// Allocate result buffers
	std::vector<int> indices_vec(m_impl->borderSize);
	std::vector<float> dists_vec(m_impl->borderSize);
	flann::Matrix<int> indices(indices_vec.data(), m_impl->borderSize, 1);
	flann::Matrix<float> dists(dists_vec.data(), m_impl->borderSize, 1);

	// std::vector<RollCoord::DiffT> rollVal(m_impl->borderSize);

	m_impl->targetIndex.knnSearch(query, indices, dists, 1, flann::SearchParams(128));

	// for (size_t i = 0; i < m_impl->borderSize; ++i)
	// {
	// 	RollCoord target = m_impl->csys.VecToRoll(m_targetMesh->vertices.at(indices_vec.at(i)));
	// 	rollVal.at(i) = queryCoords.at(i).CalcRollTo(target);
	// }
	// 
	// SmoothRoll smoothRoll;
	// smoothRoll.Init(numBuckets);
	// smoothRoll.Build(rollVal, queryCoords, smooth);
	// 
	// for (size_t i = 0; i < m_impl->borderSize; ++i)
	// {
	// 	auto roll = smoothRoll.Query(queryCoords.at(i).PrimaryAngle());
	// 	queryCoords.at(i).ApplyRoll(roll * factor);
	// }
	// 
	// // TODO: Implement
	// 
	// for (size_t i = 0; i < m_impl->borderSize; ++i)
	// {
	// 	m_mesh->vertices.at(m_impl->border.at(i)) = m_impl->csys.RollToVec(queryCoords.at(i));
	// 	//m_mesh->vertices.at(border.at(i)) = m_targetMesh->vertices.at(indices_vec.at(i));
	// }
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
