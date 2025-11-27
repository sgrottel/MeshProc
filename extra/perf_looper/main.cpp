// #include "enum_stringify.h"

#include "../../MeshProc/data/HashableEdge.h"

#include <benchmark/benchmark.h>

#include <array>
#include <format>
#include <iostream>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using meshproc::data::HashableEdge;
using LoopsVector = std::vector<std::shared_ptr<std::vector<uint32_t>>>;
using EdgesVector = std::vector<HashableEdge>;

struct TestDataSet
{
	EdgesVector m_data;

	TestDataSet(size_t seed)
	{
		std::cout << std::format("Init'ing test data set with seed {}.\n", seed);
		std::mt19937 generator(seed);

		auto addLoopEdges = [&generator, &data = m_data](size_t from, size_t to)
			{
				const size_t len = to - from;
				std::vector<int> sequence(len);
				for (int i = 0; i < len; ++i)
				{
					sequence[i] = from + i;
				}

				std::shuffle(sequence.begin(), sequence.end(), generator);

				for (size_t i = 1; i < sequence.size(); ++i)
				{
					data.push_back(HashableEdge(sequence[i - 1], sequence[i]));
				}
				data.push_back(HashableEdge(sequence.back(), sequence.front()));
			};

		addLoopEdges(0, 800);
		addLoopEdges(800, 1800);
		addLoopEdges(1800, 3400);

		std::shuffle(m_data.begin(), m_data.end(), generator);
	}

	bool IsValid(LoopsVector const& loops)
	{
		if (loops.size() != 3) return false;
		std::array<size_t, 3> sizes;
		std::transform(loops.begin(), loops.end(), sizes.begin(), [](auto p) { return p->size(); });
		std::sort(sizes.begin(), sizes.end());
		if (sizes[0] != 800) return false;
		if (sizes[1] != 1000) return false;
		if (sizes[2] != 1600) return false;
		return true;
	}
};

void LoopsFromEdgesByHalfEdges(EdgesVector const& input, LoopsVector& output);
void LoopsFromEdgesByMergeLists(EdgesVector const& input, LoopsVector& output);

static TestDataSet s_testDataSet(1337);

void BM_TestLoopsFromEdges(benchmark::State& state, void(*f)(EdgesVector const& input, LoopsVector& output))
{
	for (auto _ : state)
	{
		std::vector<std::shared_ptr<std::vector<uint32_t>>> resultLoops;
		f(s_testDataSet.m_data, resultLoops);
		benchmark::DoNotOptimize(resultLoops);
		if (!s_testDataSet.IsValid(resultLoops))
		{
			std::cerr << "Results not valid!\n";
		}
	}
}

BENCHMARK_CAPTURE(BM_TestLoopsFromEdges, HalfEdges, LoopsFromEdgesByHalfEdges)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_TestLoopsFromEdges, MergeLists, LoopsFromEdgesByMergeLists)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

void LoopsFromEdgesByHalfEdges(EdgesVector const& openEdges, LoopsVector& output)
{
	std::unordered_map<uint32_t, std::unordered_set<uint32_t>> halfEdges;
	halfEdges.reserve(openEdges.size());
	for (HashableEdge const& e : openEdges)
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

	LoopsVector* loops = &output;

	while (!halfEdges.empty())
	{
		std::shared_ptr<std::vector<uint32_t>> loop = loops->emplace_back(std::make_shared<std::vector<uint32_t>>());

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
				std::cerr << "Failed to complete loop at " << static_cast<int>(last) << '\n';
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

void LoopsFromEdgesByMergeLists(EdgesVector const& input, LoopsVector& output)
{
	auto addLoop = [&](const std::vector<size_t>& l)
		{
			auto loop = std::make_shared<std::vector<uint32_t>>();
			output.push_back(loop);
			loop->resize(l.size());
			std::transform(l.begin(), l.end(), loop->begin(), [&](size_t idx) { return static_cast<uint32_t>(idx); });
		};

	std::vector<std::vector<size_t>> polys;
	for (HashableEdge const& e : input)
	{
		auto i1 = std::find_if(polys.begin(), polys.end(), [i = e.i0](const std::vector<size_t>& l) { return l.front() == i || l.back() == i; });
		auto i2 = std::find_if(polys.begin(), polys.end(), [i = e.i1](const std::vector<size_t>& l) { return l.front() == i || l.back() == i; });

		if (i1 != polys.end())
		{
			if (i2 == i1)
			{
				// closing the loop
				addLoop(*i1);
				polys.erase(i1);
			}
			else
				if (i2 != polys.end())
				{
					// merge both lines
					if (i1->front() == e.i0)
					{
						// need to connect i1 at the front (try if we can more easily connect to i2)

						i2->reserve(i2->size() + i1->size());
						if (i2->front() == e.i1)
						{
							// need to connect i2 at the front
							// => reverse i2; then append i1 to i2
							std::reverse(i2->begin(), i2->end());
							for (auto ii = i1->begin(); ii != i1->end(); ++ii)
							{
								i2->push_back(*ii);
							}
						}
						else
						{
							// need to connect i2 at the end => append i1 to i2

							for (auto ii = i1->begin(); ii != i1->end(); ++ii)
							{
								i2->push_back(*ii);
							}
						}
						polys.erase(i1);
					}
					else
					{
						// need to connect i1 at the end
						i1->reserve(i2->size() + i1->size());
						if (i2->front() == e.i1)
						{
							// need to connect i2 at the front => append i2 to i1

							for (auto ii = i2->begin(); ii != i2->end(); ++ii)
							{
								i1->push_back(*ii);
							}
						}
						else
						{
							// need to connect i2 at the end => append reversed i2 to i1
							for (auto ii = i2->rbegin(); ii != i2->rend(); ++ii)
							{
								i1->push_back(*ii);
							}
						}

						polys.erase(i2);
					}

				}
				else
				{
					if (i1->front() == e.i0)
					{
						i1->insert(i1->begin(), e.i1);
					}
					else
					{
						i1->push_back(e.i1);
					}
				}
		}
		else
		{
			if (i2 != polys.end())
			{
				if (i2->front() == e.i1)
				{
					i2->insert(i2->begin(), e.i0);
				}
				else
				{
					i2->push_back(e.i0);
				}
			}
			else
			{
				polys.push_back(std::vector<size_t>{e.i0, e.i1});
			}
		}
	}

	if (polys.size() > 0)
	{
		std::cerr << "Open lines: " << static_cast<int>(polys.size());
		for (const auto& l : polys)
		{
			addLoop(l);
		}
	}
}
