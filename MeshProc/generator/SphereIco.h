#pragma once

#include "Icosahedron.h"

namespace meshproc
{
	namespace generator
	{

		class SphereIco : public Icosahedron
		{
		public:
			SphereIco(const sgrottel::ISimpleLog& log);

			Parameter<uint32_t> Iterations{};

			bool Invoke() override;
		};

	}
}
