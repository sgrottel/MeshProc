#pragma once

#include "Parameter.h"

namespace meshproc
{

	class ParameterBinding
	{
	public:

		struct ParamBindingBase
		{
			ParamMode m_mode;
			ParamType m_type;

			virtual ~ParamBindingBase() = default;

		protected:
			ParamBindingBase(ParamMode mode, ParamType type)
				: m_mode{ mode }, m_type{ type }
			{
			}
		};

		template<ParamType PT>
		static const ParamTypeInfo_t<PT>* GetValueSource(const ParamBindingBase* binding)
		{
			if (auto o = dynamic_cast<const ParamBinding<ParamMode::In, PT>*>(binding); o != nullptr) return &o->m_var;
			if (auto o = dynamic_cast<const ParamBinding<ParamMode::InOut, PT>*>(binding); o != nullptr) return &o->m_var;
			if (auto o = dynamic_cast<const ParamBinding<ParamMode::Out, PT>*>(binding); o != nullptr) return &o->m_var;
			return nullptr;
		}

		template<ParamType PT>
		static ParamTypeInfo_t<PT>* GetValueTarget(const ParamBindingBase* binding)
		{
			if (auto o = dynamic_cast<const ParamBinding<ParamMode::In, PT>*>(binding); o != nullptr) return &o->m_var;
			if (auto o = dynamic_cast<const ParamBinding<ParamMode::InOut, PT>*>(binding); o != nullptr) return &o->m_var;
			return nullptr;
		}

	protected:

		template<ParamMode PM, ParamType PT>
		struct ParamBinding;
	};

	template<ParamType PT>
	struct ParameterBinding::ParamBinding<ParamMode::In, PT> final : public ParamBindingBase
	{
		ParamTypeInfo_t<PT>& m_var;

		ParamBinding(const ParamTypeInfo_t<PT>& var)
			: ParamBindingBase(ParamMode::In, PT)
			, m_var{ const_cast<ParamTypeInfo_t<PT>&>(var) }
		{
		}
	};

	template<ParamType PT>
	struct ParameterBinding::ParamBinding<ParamMode::InOut, PT> final : public ParamBindingBase
	{
		ParamTypeInfo_t<PT>& m_var;

		ParamBinding(ParamTypeInfo_t<PT>& var)
			: ParamBindingBase(ParamMode::InOut, PT)
			, m_var{ var }
		{
		}
	};

	template<ParamType PT>
	struct ParameterBinding::ParamBinding<ParamMode::Out, PT> final : public ParamBindingBase
	{
		const ParamTypeInfo_t<PT>& m_var;

		ParamBinding(ParamTypeInfo_t<PT>& var)
			: ParamBindingBase(ParamMode::Out, PT)
			, m_var{ var }
		{
		}
	};

}
