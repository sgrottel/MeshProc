#include "OpenBorder.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_set>

#include <iostream>

OpenBorder::OpenBorder(sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

std::vector<std::vector<uint32_t>> OpenBorder::Find(std::shared_ptr<Mesh> const& mesh)
{
	std::unordered_set<glm::uvec2> openEdges;
	for (Triangle const& t : mesh->triangles)
	{
		for (int i = 0; i < 3; ++i)
		{
			glm::uvec2 e{ t[i], t[(i + 1) % 3] };
			if (e.x > e.y)
			{
				std::swap(e.x, e.y);
			}

			if (openEdges.find(e) == openEdges.end())
			{
				openEdges.insert(e);
			}
			else
			{
				openEdges.erase(e);
			}
		}
	}

	std::unordered_map<uint32_t, std::unordered_set<uint32_t>> halfEdges;
	halfEdges.reserve(openEdges.size());
	for (glm::uvec2 const& e : openEdges)
	{
		halfEdges[e.x].insert(e.y);
		halfEdges[e.y].insert(e.x);
	}

	auto removeEdge = [&halfEdges](uint32_t x, uint32_t y)
		{
			if (halfEdges.contains(x))
			{
				halfEdges[x].erase(y);
				if (halfEdges[x].empty())
				{
					halfEdges.erase(x);
				}
			}
			if (halfEdges.contains(y))
			{
				halfEdges[y].erase(x);
				if (halfEdges[y].empty())
				{
					halfEdges.erase(y);
				}
			}
		};

	std::vector<std::vector<uint32_t>> loops;

	while (!halfEdges.empty())
	{
		std::vector<uint32_t>& loop = loops.emplace_back();

		uint32_t next, last;
		{
			auto const& start = halfEdges.begin();
			loop.push_back(last = start->first);
			loop.push_back(next = *start->second.begin());
			removeEdge(last, next);
		}

		while (next != loop.front())
		{
			last = next;
			std::unordered_set<uint32_t>& t = halfEdges[last];
			if (t.empty())
			{
				m_log.Error("Failed to complete loop at %d", static_cast<int>(last));
			}
			next = *t.begin();
			removeEdge(last, next);
			if (next != loop.front())
			{
				loop.push_back(next);
			}
		}
	}

	return loops;
}
