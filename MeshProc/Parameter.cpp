#include "Parameter.h"

#include "AbstractCommand.h"

#include <stdexcept>

using namespace meshproc;

namespace
{

	template<unsigned int VAL>
	struct ParamTypeSelector;

	template<>
	struct ParamTypeSelector<0>
	{
		static const char* GetName(ParamType t)
		{
			if (0 == static_cast<unsigned int>(t))
			{
				return ParamTypeInfo<static_cast<ParamType>(0)>::name;
			}
			return nullptr;
		}
	};

	template<unsigned int VAL>
	struct ParamTypeSelector
	{
		static const char* GetName(ParamType t)
		{
			if (VAL == static_cast<unsigned int>(t))
			{
				return ParamTypeInfo<static_cast<ParamType>(VAL)>::name;
			}
			return ParamTypeSelector<VAL - 1>::GetName(t);
		}
	};

	template<unsigned int VAL>
	struct ParamModeSelector;

	template<>
	struct ParamModeSelector<0>
	{
		static const char* GetName(ParamMode t)
		{
			if (0 == static_cast<unsigned int>(t))
			{
				return ParamModeInfo<static_cast<ParamMode>(0)>::name;
			}
			return nullptr;
		}
	};

	template<unsigned int VAL>
	struct ParamModeSelector
	{
		static const char* GetName(ParamMode t)
		{
			if (VAL == static_cast<unsigned int>(t))
			{
				return ParamModeInfo<static_cast<ParamMode>(VAL)>::name;
			}
			return ParamModeSelector<VAL - 1>::GetName(t);
		}
	};

}

const char* meshproc::GetParamTypeName(ParamType pt)
{
	return ParamTypeSelector<static_cast<unsigned int>(ParamType::LAST) - 1>::GetName(pt);
}

const char* meshproc::GetParamModeName(ParamMode pm)
{
	return ParamModeSelector<static_cast<unsigned int>(ParamMode::LAST) - 1>::GetName(pm);
}
