#pragma once

#include "Icosahedron.h"

namespace meshproc
{
	namespace commands
	{
		namespace generator
		{

			class SphereIco : public Icosahedron
			{
			public:
				SphereIco(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const uint32_t m_iterations{ 1 };
			};

		}
	}
}
