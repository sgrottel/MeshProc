#include "MorphMeshLoop.h"

#include <SimpleLog/SimpleLog.hpp>

#include <functional>
#include <tuple>
#include <queue>
#include <unordered_set>
#include <unordered_map>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using namespace meshproc;

MorphMeshLoop::MorphMeshLoop(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Indices>("EdgeList", m_edgeList);
	AddParamBinding<ParamMode::In, ParamType::Float>("BlendArea", m_blendArea);
	AddParamBinding<ParamMode::In, ParamType::Mesh>("TargetMesh", m_targetMesh);
	AddParamBinding<ParamMode::In, ParamType::Indices>("TargetEdgeList", m_targetEdgeList);
}

bool MorphMeshLoop::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_edgeList)
	{
		Log().Error("EdgeList is empty");
		return false;
	}
	if (m_edgeList->size() < 3)
	{
		Log().Error("EdgeList must have a least three entries");
		return false;
	}
	if (!m_targetMesh)
	{
		Log().Error("TargetMesh is empty");
		return false;
	}
	if (!m_targetEdgeList)
	{
		Log().Error("TargetEdgeList is empty");
		return false;
	}
	if (m_targetEdgeList->size() < 3)
	{
		Log().Error("TargetEdgeList must have a least three entries");
		return false;
	}
	if (m_blendArea < 0)
	{
		Log().Warning("BlendArea is clamped to 0");
	}

	// ### Phase 1 ###
	// Move connect & adjust edge to the target
	if (!StitchMeshEdgeToTarget())
	{
		return false;
	}

	// ### Phase 2 ###
	// Smooth morph connected vertices in a wider area
	//
	// note:
	// source geometry was changed in phase 1; m_edgeList is no longer valid!
	// m_srcToTar was updated accordingly. Use that!
	if (!MorphMeshToTarget())
	{
		return false;
	}

	// check for isolated vertices and remove them
	RemoveIsolatedVertices();

	return true;
}

bool MorphMeshLoop::StitchMeshEdgeToTarget()
{
	// Move connect & adjust edge to the target
	auto getSrcPt = [mesh = m_mesh](size_t idx) {
		return mesh->vertices.at(idx);
		};
	auto getTarPt = [mesh = m_targetMesh](size_t idx) {
		return mesh->vertices.at(idx);
		};

	m_srcToTar.clear();

	auto connect = [&](uint32_t i, uint32_t j)
		{
			m_srcToTar[i] = j;
		};

	// first start with the absolute closest match
	Log().Detail("Find initial loop stitching match pair");
	float minDist = std::numeric_limits<float>::max();
	size_t minArgI = 0, minArgJ = 0;
	for (size_t i = 0; i < m_edgeList->size(); ++i)
	{
		glm::vec3 const& srcPt = getSrcPt(m_edgeList->at(i));
		for (size_t j = 0; j < m_targetEdgeList->size(); ++j)
		{
			glm::vec3 const& tarPt = getTarPt(m_targetEdgeList->at(j));
			const float dist = glm::distance(srcPt, tarPt);
			if (dist < minDist)
			{
				minDist = dist;
				minArgI = i;
				minArgJ = j;
			}
		}
	}

	connect(m_edgeList->at(minArgI), m_targetEdgeList->at(minArgJ));

	// build the initial sequences
	std::vector<uint32_t> srcSeq;
	srcSeq.resize(m_edgeList->size() - 1);
	for (size_t i = 1; i < m_edgeList->size(); ++i)
	{
		srcSeq.at(i - 1) = m_edgeList->at((minArgI + i) % m_edgeList->size());
	}
	std::vector<uint32_t> tarSeq;
	tarSeq.resize(m_targetEdgeList->size() - 1);
	for (size_t i = 1; i < m_targetEdgeList->size(); ++i)
	{
		tarSeq.at(i - 1) = m_targetEdgeList->at((minArgJ + i) % m_targetEdgeList->size());
	}

	if (srcSeq.size() < 3 || tarSeq.size() < 3)
	{
		Log().Error("Source boundary size or target boundary size too small");
		return false;
	}

	// check winding rule of sequences
	{
		auto const sp1 = getSrcPt(m_edgeList->at(minArgI));
		auto const sp2 = getSrcPt(srcSeq.at(srcSeq.size() / 3));
		auto const sp3 = getSrcPt(srcSeq.at((srcSeq.size() * 2) / 3));

		auto const tp1 = getTarPt(m_targetEdgeList->at(minArgJ));
		auto const tp2 = getTarPt(tarSeq.at(tarSeq.size() / 3));
		auto const tp3 = getTarPt(tarSeq.at((tarSeq.size() * 2) / 3));

		auto const sn = glm::normalize(glm::cross(sp2 - sp1, sp3 - sp2));
		auto const tn = glm::normalize(glm::cross(tp2 - tp1, tp3 - tp2));

		auto const val = glm::length(sn + tn);

		if (val < 1.0f)
		{
			std::reverse(srcSeq.begin(), srcSeq.end());
		}
	}

	// connect sequences
	Log().Detail("Initial stitching of original loop");
	{
		using seqIt = std::vector<uint32_t>::iterator;
		using seqMatchFunc = std::function<void(seqIt srcBegin, seqIt srcEnd, seqIt tarBegin, seqIt tarEnd)>;
		using seqInfo = std::tuple<seqIt, seqIt, seqIt, seqIt>;

		std::queue<seqInfo> seqQueue;

		seqMatchFunc shortSeqMatch;
		seqMatchFunc bisectSeqMatch;
		seqMatchFunc autoSeqMatch;

		shortSeqMatch = [&connect](seqIt srcBegin, seqIt srcEnd, seqIt tarBegin, seqIt tarEnd) {
			const double srcLen = static_cast<double>(std::distance(srcBegin, srcEnd));
			const size_t tarLen = std::distance(tarBegin, tarEnd);
			size_t srcPos = 0;
			for (seqIt srcIt = srcBegin; srcIt != srcEnd; ++srcIt, ++srcPos)
			{
				const size_t tarPos = std::clamp<size_t>(
					static_cast<size_t>(static_cast<double>(srcPos) * static_cast<double>(tarLen) / srcLen),
					0,
					tarLen - 1);
				connect(*srcIt, *(tarBegin + tarPos));
			}
			};

		bisectSeqMatch = [&autoSeqMatch, &getSrcPt, &getTarPt, &connect, &seqQueue](seqIt srcBegin, seqIt srcEnd, seqIt tarBegin, seqIt tarEnd) {
			const size_t srcLen = std::distance(srcBegin, srcEnd);
			const size_t tarLen = std::distance(tarBegin, tarEnd);

			const seqIt sB = srcBegin + srcLen / 4;
			const seqIt sE = srcBegin + srcLen * 3 / 4;

			const seqIt tB = tarBegin + tarLen / 4;
			const seqIt tE = tarBegin + tarLen * 3 / 4;

			seqIt sS = srcBegin + srcLen / 2, tS = tarBegin + tarLen / 2;
			float minDist = std::numeric_limits<float>::max();
			for (seqIt sI = sB; sI != sE; ++sI)
			{
				glm::vec3 const& srcPt = getSrcPt(*sI);
				for (seqIt tI = tB; tI != tE; ++tI)
				{
					glm::vec3 const& tarPt = getTarPt(*tI);
					const float dist = glm::distance(srcPt, tarPt);
					if (dist < minDist)
					{
						minDist = dist;
						sS = sI;
						tS = tI;
					}
				}
			}

			connect(*sS, *tS);
			seqQueue.push(std::make_tuple(srcBegin, sS, tarBegin, tS));
			seqQueue.push(std::make_tuple(std::next(sS), srcEnd, std::next(tS), tarEnd));
			};

		autoSeqMatch = [&shortSeqMatch, &bisectSeqMatch](seqIt srcBegin, seqIt srcEnd, seqIt tarBegin, seqIt tarEnd) {
			const size_t srcLen = std::distance(srcBegin, srcEnd);
			const size_t tarLen = std::distance(tarBegin, tarEnd);

			if (srcLen > 3 && tarLen > 3)
			{
				bisectSeqMatch(srcBegin, srcEnd, tarBegin, tarEnd);
			}
			else
			{
				shortSeqMatch(srcBegin, srcEnd, tarBegin, tarEnd);
			}
			};

		seqQueue.push(std::make_tuple(srcSeq.begin(), srcSeq.end(), tarSeq.begin(), tarSeq.end()));

		while (!seqQueue.empty())
		{
			auto seq = seqQueue.front();
			seqQueue.pop();

			autoSeqMatch(std::get<0>(seq), std::get<1>(seq), std::get<2>(seq), std::get<3>(seq));
		}
	}

	// fix post-move edge geometry
	Log().Detail("Post-stitching fixing geometry");
	{
		// remove degenerated triangles
		// close gap holes between mesh border and stitched border
		std::unordered_set<data::HashableEdge> tarEdges;
		for (size_t ti = 0; ti < m_targetEdgeList->size(); ++ti)
		{
			const uint32_t i1 = m_targetEdgeList->at(ti);
			const uint32_t i2 = m_targetEdgeList->at((ti + 1) % m_targetEdgeList->size());
			tarEdges.insert(data::HashableEdge{ i1, i2 });
		}

		for (size_t si = 0; si < m_edgeList->size(); ++si)
		{
			const uint32_t is1 = m_edgeList->at(si);
			const uint32_t is2 = m_edgeList->at((si + 1) % m_edgeList->size());

			const uint32_t it1 = m_srcToTar.at(is1);
			const uint32_t it2 = m_srcToTar.at(is2);

			if (it1 == it2)
			{
				// replace all use of is1 with is2
				for (auto& t : m_mesh->triangles)
				{
					for (size_t j = 0; j < 3; ++j)
					{
						if (t[j] == is1)
						{
							t[j] = is2;
						}
					}
				}
				m_srcToTar.erase(is1);
				continue;
			}

			data::HashableEdge testEdge{ it1, it2 };
			if (!tarEdges.contains(testEdge))
			{
				const size_t ts = m_targetEdgeList->size();
				std::vector<uint32_t> tidxs;
				for (size_t tii = 0; tii < ts * 2; ++tii)
				{
					uint32_t tv = m_targetEdgeList->at(tii % m_targetEdgeList->size());
					if (!tidxs.empty())
					{
						tidxs.push_back(tv);
					}
					if (tv == it1 || tv == it2)
					{
						if (tidxs.empty())
						{
							tidxs.push_back(tv);
						}
						else
						{
							break;
						}
					}
				}
				if (tidxs.front() == it1)
				{
					assert(tidxs.back() == it2);
				}
				else
				{
					assert(tidxs.front() == it2);
					assert(tidxs.back() == it1);
					std::reverse(tidxs.begin(), tidxs.end());
				}

				auto oTI = m_mesh->triangles.end();
				for (auto tI = m_mesh->triangles.begin(); tI != m_mesh->triangles.end(); ++tI)
				{
					if (tI->HasIndex(is1) && tI->HasIndex(is2))
					{
						if (oTI == m_mesh->triangles.end())
						{
							oTI = tI;
						}
						else
						{
							Log().Error("Multiple triangles claim to span the stitch gap hole");
						}
					}
				}
				data::Triangle oT = *oTI;
				m_mesh->triangles.erase(oTI);
				while (oT[0] == is1 || oT[0] == is2)
				{
					uint32_t t = oT[0];
					oT[0] = oT[1];
					oT[1] = oT[2];
					oT[2] = t;
				}
				if (oT[1] == is1)
				{
					assert(oT[2] == is2);
				}
				else
				{
					assert(oT[1] == is2);
					assert(oT[2] == is1);
					std::reverse(tidxs.begin(), tidxs.end());
				}

				// inject vertices
				std::vector<uint32_t> sidxs;
				sidxs.push_back(is1);
				for (size_t i = 1; i < tidxs.size() - 1; ++i)
				{
					float b = static_cast<float>(i) / static_cast<float>(tidxs.size() - 1);
					assert(0.0f < b && b < 1.0f);
					float a = 1.0f - b;
					glm::vec3 vi = m_mesh->vertices.at(is1) * a + m_mesh->vertices.at(is2) * b;
					uint32_t isi = static_cast<uint32_t>(m_mesh->vertices.size());
					m_srcToTar.insert(std::make_pair(isi, tidxs.at(i)));
					sidxs.push_back(isi);
					m_mesh->vertices.push_back(vi);
				}
				sidxs.push_back(is2);

				for (size_t i = 1; i < sidxs.size(); ++i)
				{
					m_mesh->triangles.push_back(data::Triangle{ oT[0], sidxs.at(i - 1), sidxs.at(i) });
				}
			}
		}

		auto newEnd = std::remove_if(m_mesh->triangles.begin(), m_mesh->triangles.end(), [](auto const& t) {
			return t[0] == t[1] || t[0] == t[2] || t[1] == t[2];
			});
		if (newEnd != m_mesh->triangles.end())
		{
			m_mesh->triangles.erase(newEnd, m_mesh->triangles.end());
		}
	}

	return true;
}

bool MorphMeshLoop::MorphMeshToTarget()
{
	// Smooth morph connected vertices in a wider area
	Log().Detail("Morphing mesh");
	//
	// note:
	// source geometry was changed in phase 1; m_edgeList is no longer valid!
	// m_srcToTar was updated accordingly. Use that!
	const float blendArea = std::max(0.0f, m_blendArea);

	struct MoveInfo {
		float dist{ std::numeric_limits<float>::max() };
		glm::vec3 move{ 0.0f, 0.0f, 0.0f };
		float weight{ 0.0f };
	};

	// result
	std::unordered_map<uint32_t, MoveInfo> moveInfo;

	// triangles to search
	std::unordered_set<uint32_t> triIdxs;
	std::unordered_set<uint32_t> triIdxsSel;

	triIdxs.reserve(m_mesh->triangles.size());
	for (size_t i = 0; i < m_mesh->triangles.size(); ++i)
	{
		triIdxs.insert(static_cast<uint32_t>(i));
	}

	// moving border vertices
	std::unordered_set<uint32_t> verIdxs; // front
	std::unordered_set<uint32_t> verIdxsSel;

	// initialization
	for (auto const& s2t : m_srcToTar)
	{
		verIdxs.insert(s2t.first);
		moveInfo.insert(
			std::make_pair(
				s2t.first,
				MoveInfo{
					.dist = 0.0f,
					.move = m_targetMesh->vertices.at(s2t.second) - m_mesh->vertices.at(s2t.first),
					.weight = 1.0f
				}
			));
	}

	// iterate:
	while (!verIdxs.empty())
	{
		// select new border triangles & vertices
		triIdxsSel.clear();
		verIdxsSel.clear();
		for (uint32_t ti : triIdxs)
		{
			data::Triangle const& t = m_mesh->triangles.at(ti);

			bool vertVal[3] = {
				verIdxs.contains(t[0]),
				verIdxs.contains(t[1]),
				verIdxs.contains(t[2])
			};

			bool const sel = vertVal[0] || vertVal[1] || vertVal[2];
			if (!sel) continue;

			triIdxsSel.insert(ti);

			glm::vec3 move{ 0.0f, 0.0f, 0.0f };
			{
				float weight{ 0.0f };
				for (size_t j = 0; j < 3; j++)
				{
					if (!vertVal[j]) continue;
					move += moveInfo[t[j]].move / moveInfo[t[j]].weight;
					weight += 1.0f;
				}
				assert(weight > 0.0f);
				move /= weight;
			}

			for (size_t j = 0; j < 3; j++)
			{
				if (vertVal[j]) continue;

				if (verIdxsSel.insert(t[j]).second)
				{
					moveInfo.insert(std::make_pair(t[j], MoveInfo{}));
				}

				auto& mi = moveInfo.at(t[j]);

				mi.move += move;
				mi.weight += 1.0f;
				glm::vec3 const& pj = m_mesh->vertices.at(t[j]);
				for (size_t k = 0; k < 3; k++)
				{
					if (!vertVal[k]) continue;
					assert(j != k);
					glm::vec3 const& pk = m_mesh->vertices.at(t[k]);
					float const distViaK = moveInfo.at(t[k]).dist + glm::distance(pk, pj);
					if (distViaK < mi.dist)
					{
						mi.dist = distViaK;
					}
				}
			}
		}

		for (uint32_t ti : triIdxsSel)
		{
			triIdxs.erase(ti);
		}

		std::erase_if(verIdxsSel, [&blendArea, &moveInfo](uint32_t vi) { return moveInfo.at(vi).dist > blendArea; });

		std::swap(verIdxs, verIdxsSel);
	}

	Log().Detail("Applying morphing move info");
	for (auto const& mip : moveInfo)
	{
		const float dist = mip.second.dist;
		if (dist > blendArea) continue;

		const float distFactor = 1.0f - dist / blendArea;
		const float distWeight = distFactor * distFactor;

		const float weight = distWeight / mip.second.weight;

		m_mesh->vertices.at(mip.first) += mip.second.move * weight;
	}

	return true;
}

void MorphMeshLoop::RemoveIsolatedVertices()
{
	std::unordered_map<uint32_t, uint32_t> verts;
	verts.reserve(m_mesh->vertices.size());
	for (uint32_t i = 0; i < static_cast<uint32_t>(m_mesh->vertices.size()); ++i)
	{
		verts.insert(std::make_pair(i, 0xffffffffu));
	}
	for (auto const& t : m_mesh->triangles)
	{
		for (size_t j = 0; j < 3; j++)
		{
			verts.at(t[j]) = t[j];
		}
	}

	std::vector<uint32_t> vtd;
	uint32_t off = 0;
	for (uint32_t i = 0; i < static_cast<uint32_t>(m_mesh->vertices.size()); ++i)
	{
		uint32_t& v = verts.at(i);
		if (v == 0xffffffffu)
		{
			vtd.push_back(i);
			off++;
		}
		else
		{
			assert(v >= off);
			v -= off;
		}
	}
	if (off > 0)
	{
		Log().Detail("Removing %d isolated/deprecated vertices", off);

		for (auto& t : m_mesh->triangles)
		{
			for (size_t j = 0; j < 3; j++)
			{
				t[j] = verts.at(t[j]);
			}
		}
		std::reverse(vtd.begin(), vtd.end());
		for (uint32_t vtdi : vtd)
		{
			m_mesh->vertices.erase(m_mesh->vertices.begin() + vtdi);
		}
	}
}
