#include "DeleteTriangles.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_set>
#include <unordered_map>

using namespace meshproc;

DeleteTriangles::DeleteTriangles(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Indices>("TrianglesList", m_triList);
}

bool DeleteTriangles::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("InputMesh is empty");
		return false;
	}
	if (!m_triList)
	{
		Log().Error("TrianglesList is empty");
		return false;
	}

	// delete triangles
	{
		std::unordered_set<uint32_t> idx(m_triList->begin(), m_triList->end());
		std::erase_if(m_mesh->triangles, [&idx, i = 0](const auto&) mutable {
			return idx.contains(i++);
			});
	}

	// delete isolated vertices
	{
		std::unordered_set<uint32_t> idx;
		idx.reserve(m_mesh->vertices.size());
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_mesh->vertices.size()); ++i)
		{
			idx.insert(i);
		}

		for (auto const& t : m_mesh->triangles)
		{
			for (size_t j = 0; j < 3; ++j)
			{
				idx.erase(t[j]);
			}
		}

		if (!idx.empty())
		{
			// there are indices to remove
			std::unordered_map<uint32_t, uint32_t> reindex;

			reindex.reserve(m_mesh->vertices.size() - idx.size());
			uint32_t d = 0;
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_mesh->vertices.size()); ++i)
			{
				if (idx.contains(i))
				{
					d++;
				}
				else
				{
					reindex.insert(std::make_pair(i, i - d));
				}
			}

			std::erase_if(m_mesh->vertices, [&idx, i = 0](const auto&) mutable {
				return idx.contains(i);
				});

			for (auto& t : m_mesh->triangles)
			{
				for (size_t j = 0; j < 3; ++j)
				{
					t[j] = reindex.at(t[j]);
				}
			}
		}
	}

	return true;
}
