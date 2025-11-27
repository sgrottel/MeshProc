#pragma once

#include "data/HashableEdge.h"
#include "Parameter.h"

#include <SimpleLog/SimpleLog.hpp>

namespace meshproc
{
	namespace algo
	{

		template<typename EdgesT>
		void LoopsFromEdges(EdgesT const& edges, ParamTypeInfo_t<ParamType::MultiIndices>& outLoops, sgrottel::ISimpleLog const& log)
		{
			std::unordered_map<uint32_t, std::unordered_set<uint32_t>> halfEdges;
			halfEdges.reserve(edges.size());
			for (data::HashableEdge const& e : edges)
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

			if (outLoops)
			{
				outLoops->clear();
			}
			else
			{
				outLoops = std::make_shared<std::vector<ParamTypeInfo_t<ParamType::Indices>>>();
			}

			while (!halfEdges.empty())
			{
				ParamTypeInfo_t<ParamType::Indices> loop = outLoops->emplace_back(std::make_shared<std::vector<uint32_t>>());

				uint32_t next, last;
				{
					auto const& start = halfEdges.begin();
					loop->push_back(last = start->first);
					loop->push_back(next = *start->second.begin());
					removeEdge(last, next);
				}

				while (next != loop->front())
				{
					last = next;
					std::unordered_set<uint32_t>& t = halfEdges[last];
					if (t.empty())
					{
						log.Error("Failed to complete loop at %d", static_cast<int>(last));
					}
					next = *t.begin();
					removeEdge(last, next);
					if (next != loop->front())
					{
						loop->push_back(next);
					}
				}
			}
		}

	}
}
