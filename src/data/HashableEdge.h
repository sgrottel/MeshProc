#pragma once

#include <cstdint>
#include <utility>

namespace meshproc
{
	namespace data
	{
		class HashableEdge
		{
		public:
			inline HashableEdge(uint32_t v0, uint32_t v1)
				: m_idx{ v0, v1 }
			{
			}
			HashableEdge(const HashableEdge& src) = default;
			HashableEdge(HashableEdge&& src) = default;
			HashableEdge& operator=(const HashableEdge& src) = default;
			HashableEdge& operator=(HashableEdge&& src) = default;

			inline bool operator==(const HashableEdge& rhs) const
			{
				return (m_idx[0] == rhs.m_idx[0] && m_idx[1] == rhs.m_idx[1])
					|| (m_idx[0] == rhs.m_idx[1] && m_idx[1] == rhs.m_idx[0]);
			}
			inline bool operator!=(const HashableEdge& rhs) const
			{
				return !(*this == rhs);
			}

			inline bool Has(uint32_t i) const noexcept
			{
				return i0 == i || i1 == i;
			}

			union {
				uint32_t m_idx[2];
				struct {
					uint32_t i0, i1;
				};
				struct {
					uint32_t start, end;
				};
				struct {
					uint32_t x, y;
				};
			};
		};

		static_assert(sizeof(HashableEdge) == sizeof(uint32_t) * 2);
	}
}

namespace std
{
	template<>
	struct hash<meshproc::data::HashableEdge>
	{
		inline size_t operator()(const meshproc::data::HashableEdge& edge) const noexcept
		{
			size_t h0 = std::hash<uint32_t>{}(std::min<uint32_t>(edge.m_idx[0], edge.m_idx[1]));
			size_t h1 = std::hash<uint32_t>{}(std::max<uint32_t>(edge.m_idx[0], edge.m_idx[1]));
			return h0 ^ (h1 << 16 | h1 >> 16);
		}
	};
}
