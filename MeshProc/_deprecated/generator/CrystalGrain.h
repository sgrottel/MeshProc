#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>

namespace meshproc
{
	namespace generator
	{

		class CrystalGrain : public AbstractCommand
		{
		public:
			CrystalGrain(const sgrottel::ISimpleLog& log);

			bool Invoke() override;

		protected:
			std::shared_ptr<data::Mesh> m_mesh{};
			const uint32_t m_randomSeed{ static_cast<uint32_t>(time(0)) };
			// 0 -- sphere
			// 1 -- squircle-like
			const uint32_t m_samplingShape{ 1 };
			const uint32_t m_numFaces{ 50 };
			const float m_sizeX{ 3.0f };
			const float m_sizeY{ 3.0f };
			const float m_sizeZ{ 1.0f };
			const float m_radSigma{ 0.005f };
		};

	}
}
