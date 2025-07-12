#include "OpenBorder.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#include <unordered_map>

#include <iostream>

using namespace meshproc;

OpenBorder::OpenBorder(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::Out, ParamType::MultiVertexSelection>("EdgeLists", m_edgeLists);
}

bool OpenBorder::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (m_mesh->triangles.empty())
	{
		Log().Error("Mesh has no triangles");
		return false;
	}

	Log().Detail("Detecting open border edges");

	std::unordered_set<data::HashableEdge> openEdges = m_mesh->CollectOpenEdges();

	// TODO: should be merged with the code in Extract2DLoops and moved to a utility function
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

	ParamTypeInfo_t<ParamType::MultiVertexSelection> loops = std::make_shared<std::vector<ParamTypeInfo_t<ParamType::VertexSelection>>>();

	while (!halfEdges.empty())
	{
		ParamTypeInfo_t<ParamType::VertexSelection> loop = loops->emplace_back(std::make_shared<std::vector<uint32_t>>());

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
				Log().Error("Failed to complete loop at %d", static_cast<int>(last));
			}
			next = *t.begin();
			removeEdge(last, next);
			if (next != loop->front())
			{
				loop->push_back(next);
			}
		}
	}

	Log().Detail("Found %d open border loops", static_cast<int>(loops->size()));

	m_edgeLists = loops;

	return true;
}
