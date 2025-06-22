#pragma once

#include "AbstractType.h"

#include <memory>
#include <vector>

namespace meshproc
{
	namespace lua
	{
		template<typename TINNERVAR, typename TIMPL>
		class AbstractMultiType : public AbstractType<std::vector<std::shared_ptr<TINNERVAR>>, TIMPL>
		{
		protected:
			AbstractMultiType(Runner& owner)
				: AbstractType<std::vector<std::shared_ptr<TINNERVAR>>, TIMPL>{ owner }
			{};
		};
	}
}
