#include "MatchShape2D.h"

#include <SimpleLog/SimpleLog.hpp>

#undef min
#undef max

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/pca.hpp>
#include <glm/gtx/vector_angle.hpp> 

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <iostream>
#include <stdexcept>
#include <numbers>
#include <numeric>
#include <vector>

using namespace meshproc;

namespace
{

	std::vector<glm::vec2> ToPointcloud(std::shared_ptr<data::Shape2D> const& shape, const float precision)
	{
		std::vector<glm::vec2> pts;

		auto add = [&](glm::vec2 const& v1, glm::vec2 const& v2)
			{
				const size_t c = 1 + static_cast<size_t>((glm::length(v2 - v1) / precision) + 0.5);
				for (size_t i = 0; i < c; ++i)
				{
					const float b = static_cast<float>(i) / static_cast<float>(c);
					const float a = 1.0f - b;
					const auto v = v1 * a + v2 * b;
					pts.push_back({ v.x, v.y });
				}
			};

		for (auto const& loop : shape->loops)
		{
			const size_t c = loop.second.size();
			if (c < 2) continue;
			if (c == 2)
			{
				add(loop.second.at(0), loop.second.at(1));
				const auto& v = loop.second.at(1);
				pts.push_back({ v.x, v.y });
			}
			for (size_t i = 0; i < c; ++i)
			{
				add(loop.second.at(i), loop.second.at((i + 1) % c));
			}
		}

		return pts;
	}

	glm::vec2 RemoveCenter(std::vector<glm::vec2>& pts)
	{
		glm::dvec2 c{ 0.0, 0.0 };
		for (glm::vec2 const& v : pts)
		{
			c += v;
		}
		c /= static_cast<double>(pts.size());
		for (glm::vec2& v : pts)
		{
			v -= c;
		}
		return c;
	}

	struct PCARes
	{
		glm::vec2 evec1; // big
		float eval1;
		glm::vec2 evec2; // small
		float eval2;
	};

	PCARes CalcPCA(std::vector<glm::vec2> const& pts)
	{
		glm::mat2 covarMat = glm::computeCovarianceMatrix(pts.data(), pts.size());

		glm::vec2 evals;
		glm::mat2 evecs;
		int evcnt = glm::findEigenvaluesSymReal(covarMat, evals, evecs);
		if (evcnt != 2) throw std::runtime_error("Failed to find two eigenvalues");

		PCARes res{
			.evec1 = evecs[0],
			.eval1 = evals[0],
			.evec2 = evecs[1],
			.eval2 = evals[1]
		};

		if (res.eval1 < res.eval2)
		{
			std::swap(res.eval1, res.eval2);
			std::swap(res.evec1, res.evec2);
		}

		return res;
	}

	std::vector<float> ProjectedAbs(std::vector<glm::vec2> const& pts, const glm::vec2 v)
	{
		std::vector<float> d(pts.size(), 0.0f);
		std::transform(pts.begin(), pts.end(), d.begin(), [dir = glm::normalize(v)](glm::vec2 const& i) { return std::abs(glm::dot(i, dir)); });
		return d;
	}

	float GetPercentile(std::vector<float>& data, float percentile)
	{
		const size_t n = data.size();
		const size_t p90_index = static_cast<size_t>(std::ceil(percentile * n)) - 1;

		std::nth_element(data.begin(), data.begin() + p90_index, data.end());
		return data[p90_index];
	}

	double CalcDiff(std::vector<glm::vec2> const& targetPts, std::vector<glm::vec2> const& sourcePts)
	{
		double dist = 0.0;
		for (auto const& s : sourcePts)
		{
			double d = std::numeric_limits<float>::max();
			for (auto const& t : targetPts)
			{
				double std = std::abs(glm::distance(t, s));
				if (std < d)
				{
					d = std;
				}
			}
			dist += d;
		}
		dist /= sourcePts.size();
		return dist;
	}

}

MatchShape2D::MatchShape2D(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Shape2D>("ShapeTarget", m_shapeTarget);
	AddParamBinding<ParamMode::In, ParamType::Shape2D>("Shape", m_shape);
	AddParamBinding<ParamMode::InOut, ParamType::Mat4>("Transform", m_transform);
}

bool MatchShape2D::Invoke()
{
	if (!m_shape)
	{
		Log().Error("Shape not set");
		return false;
	}
	if (!m_shapeTarget)
	{
		Log().Error("ShapeTarget not set");
		return false;
	}

	constexpr float precs = 0.02f; // TODO: estimate precision form average edge length in both shapes

	auto targetPts = ToPointcloud(m_shapeTarget, precs);
	auto targetCenter = RemoveCenter(targetPts);
	auto targetPca = CalcPCA(targetPts);
	auto targetXs = ProjectedAbs(targetPts, targetPca.evec1);
	auto targetYs = ProjectedAbs(targetPts, targetPca.evec2);

	auto sourcePts = ToPointcloud(m_shape, precs);
	auto sourceCenter = RemoveCenter(sourcePts);
	auto sourcePca = CalcPCA(sourcePts);
	auto sourceXs = ProjectedAbs(sourcePts, sourcePca.evec1);
	auto sourceYs = ProjectedAbs(sourcePts, sourcePca.evec2);

	auto calcScale = [&](const float percentile)
		{
			auto targetX = GetPercentile(targetXs, percentile);
			auto sourceX = GetPercentile(sourceXs, percentile);

			float scaleX = targetX / sourceX;

			auto targetY = GetPercentile(targetYs, percentile);
			auto sourceY = GetPercentile(sourceYs, percentile);

			float scaleY = targetY / sourceY;

			return std::max(scaleX, scaleY);
			// return (scaleX + scaleY) * 0.5f;
			// return (std::max(scaleX, scaleY) * 2.0f + scaleX + scaleY) / 4.0f;
		};

	std::vector<float> scaleFactors{
		calcScale(0.50f),
		calcScale(0.75f),
		calcScale(0.90f),
		calcScale(0.95f),
		calcScale(0.97f),
		calcScale(0.99f)
		};
	std::sort(scaleFactors.begin(), scaleFactors.end());
	scaleFactors.erase(scaleFactors.begin());

	const float scaleFactor = std::reduce(scaleFactors.begin(), scaleFactors.end()) / scaleFactors.size();
	Log().Detail("Scaling factor = %f", scaleFactor);

	float angle = glm::angle(targetPca.evec1, sourcePca.evec1);

	for (glm::vec2& p : sourcePts)
	{
		p = glm::rotateZ(glm::vec3{ p, 0.0 }, -angle) * scaleFactor;
	}

	const double diff1 = CalcDiff(targetPts, sourcePts);

	for (glm::vec2& p : sourcePts)
	{
		p *= -1;
	}

	const double diff2 = CalcDiff(targetPts, sourcePts);
	if (diff2 < diff1)
	{
		angle += std::numbers::pi_v<float>;
	}

	auto translate1 = glm::translate(glm::identity<glm::mat4>(), glm::vec3{ -sourceCenter, 0.0f });
	auto rotate = glm::rotate(glm::identity<glm::mat4>(), -angle, glm::vec3{ 0.0f, 0.0f, 1.0f });
	auto scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3{ scaleFactor, scaleFactor, scaleFactor });
	auto translate2 = glm::translate(glm::identity<glm::mat4>(), glm::vec3{ targetCenter, 0.0f });

	m_transform = translate2 * scale * rotate * translate1;

	return true;
}
