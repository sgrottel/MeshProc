#pragma once

#include "Parameter.h"
#include "ParameterBinding.h"

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
		AbstractCommand(const sgrottel::ISimpleLog& log);

		virtual bool Invoke() = 0;

		void LogInfo(const sgrottel::ISimpleLog& log, bool verbose) const;

		inline std::shared_ptr<ParameterBinding::ParamBindingBase> GetParam(const std::string& name) const
		{
			return m_paramsRefs.GetParam(name);
		}

		inline const std::string& TypeName() const
		{
			return m_typeName;
		}

		// only to be called from within CommandFactory
		void InitTypeName(std::string const& name);

	protected:

		// To be called during Ctor to register all parameter object available to the framework
		template<ParamMode PM, ParamType PT, typename T>
		inline AbstractCommand& AddParamBinding(const std::string& name, T& var)
		{
			static_assert(std::is_const_v<T> == (PM == ParamMode::In), "In parameters must use `const` variables. InOut and Out parameters must not use `const` variables.");
			m_paramsRefs.AddParamBinding<PM, PT, T>(name, var);
			return *this;
		}

		inline const sgrottel::ISimpleLog& Log() const noexcept
		{
			return m_log;
		}

	private:

		class ParamBindingRefs : public ParameterBinding
		{
		public:

			inline void LogInfo(const sgrottel::ISimpleLog& log, bool verbose) const;

			template<ParamMode PM, ParamType PT, typename T>
			inline void AddParamBinding(const std::string& name, T& var);

			std::shared_ptr<ParamBindingBase> GetParam(const std::string& name) const;

		private:

			std::unordered_map<std::string, std::shared_ptr<ParamBindingBase>> m_params;
		};

		const sgrottel::ISimpleLog& m_log;
		std::string m_typeName;
		ParamBindingRefs m_paramsRefs;
	};

	template<ParamMode PM, ParamType PT, typename T>
	void AbstractCommand::ParamBindingRefs::AddParamBinding(const std::string& name, T& var)
	{
		if (m_params.find(name) != m_params.end())
		{
			throw std::logic_error("Cannot add param. Another param with same name already registered");
		}
		m_params[name] = std::make_shared<struct ParamBinding<PM, PT>>(var);
	}

}
