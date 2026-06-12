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

#ifndef REGISTER_COMMAND

#define REGISTER_COMMAND_ARG_AS_TYPE1(a) ::##a
#define REGISTER_COMMAND_ARG_AS_TYPE2(a,b) ::##a##::##b
#define REGISTER_COMMAND_ARG_AS_TYPE3(a,b,c) ::##a##::##b##::##c
#define REGISTER_COMMAND_ARG_AS_TYPE4(a,b,c,d) ::##a##::##b##::##c##::##d
#define REGISTER_COMMAND_ARG_AS_TYPE5(a,b,c,d,e) ::##a##::##b##::##c##::##d##::##e

#define REGISTER_COMMAND_SELECT(_1,_2,_3,_4,_5,NAME,...) NAME
#define REGISTER_COMMAND_EVALNAME(a) a

#define REGISTER_COMMAND_ARG_AS_TYPE(...) REGISTER_COMMAND_EVALNAME(REGISTER_COMMAND_SELECT(__VA_ARGS__, REGISTER_COMMAND_ARG_AS_TYPE5, REGISTER_COMMAND_ARG_AS_TYPE4, REGISTER_COMMAND_ARG_AS_TYPE3, REGISTER_COMMAND_ARG_AS_TYPE2, REGISTER_COMMAND_ARG_AS_TYPE1)(__VA_ARGS__))

#define REGISTER_COMMAND(...) template<> struct ::meshproc::commands::_utils::Guard<__COUNTER__ + 1> : public ::meshproc::commands::_utils::GuardCounted<__COUNTER__, REGISTER_COMMAND_ARG_AS_TYPE(__VA_ARGS__)> { };

#endif

namespace meshproc
{
	namespace commands
	{

		class AbstractCommand
		{
		public:
			static constexpr const char* InvokeMethodName = "invoke";

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
			if (name == InvokeMethodName)
			{
				throw std::logic_error("Cannot add param. This is a reserved name.");
			}
			if (m_params.find(name) != m_params.end())
			{
				throw std::logic_error("Cannot add param. Another param with same name already registered");
			}
			auto newParamBinding = std::make_shared<struct ParamBinding<PM, PT>>(var);
			newParamBinding->m_idx = m_params.size();
			m_params[name] = newParamBinding;
		}

		namespace _utils
		{
			template<int i>
			struct Guard;

#ifndef REGISTER_COMMAND_GUARD_0
#define REGISTER_COMMAND_GUARD_0
			template<>
			struct Guard<0> {
				constexpr static int val = 0;
				void CollectNames([[maybe_unused]] std::string& str) { }
			};
#endif

			template<int i>
			struct Guard {
				constexpr static int val = Guard<i - 1>::val;
				void CollectNames(std::string& str)
				{
					Guard<i - 1>::CollectNames(str);
				}
			};

			template<int i, typename CmdT>
			struct GuardCounted {
				constexpr static int val = Guard<i - 1>::val + 1;
				void CollectNames(std::string& str)
				{
					Guard<i - 1>::CollectNames(str);
					str += typeid(CmdT).name();
					str += '\n';
				}
			};
		}

	}
}
