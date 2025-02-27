#pragma once

#include "Parameter.h"

#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{

	class AbstractCommand
	{
	public:

		struct ParamBindingBase
		{
			ParamMode m_mode;
			ParamType m_type;
		protected:
			ParamBindingBase(ParamMode mode, ParamType type)
				: m_mode{ mode }, m_type{ type }
			{
			}
		};

		template<ParamMode PM, ParamType PT>
		struct ParamBinding;

		AbstractCommand(const sgrottel::ISimpleLog& log);

		virtual bool Invoke() = 0;

		void LogInfo(const sgrottel::ISimpleLog& log, bool verbose) const;

		// Framework function, only to be called by `MeshProgram`
		// TODO: I don't like this
		//			Implement a new "class ParamBinding",
		//			use as object here (composition, not inheritance), and
		//			"somehow" allow the MeshProgram to access the "ParamBinding"
		//			There implement a generic ParamDataStore, to be used for variables and consts.
		std::shared_ptr<ParamBindingBase> AccessParam(const std::string& name) const;

	protected:

		template<ParamMode PM, ParamType PT, typename T>
		inline AbstractCommand& AddParamBinding(const std::string& name, T& var)
		{
			static_assert(std::is_const_v<T> == (PM == ParamMode::In), "In parameters must use `const` variables");
			return AddParam(name, ParamBinding<PM, PT>{ var });
		}

		inline const sgrottel::ISimpleLog& Log() const noexcept
		{
			return m_log;
		}

	private:

		// To be called during Ctor to register all parameter object available to the framework
		template<ParamMode PM, ParamType PT>
		AbstractCommand& AddParam(const std::string& name, struct ParamBinding<PM, PT>&& var);

		const sgrottel::ISimpleLog& m_log;
		std::unordered_map<std::string, std::shared_ptr<ParamBindingBase>> m_params;
	};

	template<ParamType PT>
	struct AbstractCommand::ParamBinding<ParamMode::In, PT> final : public ParamBindingBase
	{
		ParamTypeInfo_t<PT>& m_var;

		ParamBinding(const ParamTypeInfo_t<PT>& var)
			: ParamBindingBase(ParamMode::In, PT)
			, m_var{ const_cast<ParamTypeInfo_t<PT>&>(var) }
		{
		}
	};

	template<ParamType PT>
	struct AbstractCommand::ParamBinding<ParamMode::InOut, PT> final : public ParamBindingBase
	{
		ParamTypeInfo_t<PT>& m_var;

		ParamBinding(ParamTypeInfo_t<PT>& var)
			: ParamBindingBase(ParamMode::InOut, PT)
			, m_var{ var }
		{
		}
	};

	template<ParamType PT>
	struct AbstractCommand::ParamBinding<ParamMode::Out, PT> final : public ParamBindingBase
	{
		ParamTypeInfo_t<PT>& m_var;

		ParamBinding(ParamTypeInfo_t<PT>& var)
			: ParamBindingBase(ParamMode::Out, PT)
			, m_var{ var }
		{
		}
	};

	template<ParamMode PM, ParamType PT>
	AbstractCommand& AbstractCommand::AddParam(const std::string& name, struct ParamBinding<PM, PT>&& var)
	{
		if (m_params.find(name) != m_params.end())
		{
			throw std::logic_error("Cannot add param. Another param with same name already registered");
		}
		m_params[name] = std::make_shared<struct ParamBinding<PM, PT>>(var);
		return *this;
	}

}
