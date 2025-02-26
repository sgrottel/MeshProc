#include "OpenBorder.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#include <unordered_map>
#include <unordered_set>

#include <iostream>

using namespace meshproc;

OpenBorder::OpenBorder(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParam("Mesh", Mesh);
	AddParam("EdgeLists", EdgeLists);
}

bool OpenBorder::Invoke()
{
	Log().Detail("Detecting open border edges");

	std::unordered_set<data::HashableEdge> openEdges;
	for (data::Triangle const& t : Mesh.Get()->triangles)
	{
		for (int i = 0; i < 3; ++i)
		{
			data::HashableEdge e{ t.HashableEdge(i) };
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
	for (data::HashableEdge const& e : openEdges)
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
				Log().Error("Failed to complete loop at %d", static_cast<int>(last));
			}
			next = *t.begin();
			removeEdge(last, next);
			if (next != loop.front())
			{
				loop.push_back(next);
			}
		}
	}

	Log().Detail("Found %d open border loops", static_cast<int>(loops.size()));

	EdgeLists.Put() = loops;

	return true;
}
