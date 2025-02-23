#pragma once

#include "generator/Icosahedron.h"

namespace meshproc
{
	namespace generator
	{

		class SphereIco : public Icosahedron
		{
		public:
			SphereIco(const sgrottel::ISimpleLog& log);

			Parameter<uint32_t, ParamType::In> Iterations{};

			bool Invoke() override;
		};

	}
}
