#pragma once

#include "Parameter.h"

#include <string>
#include <tuple>
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

		void PreInvoke();
		virtual bool Invoke() = 0;
		void PostInvoke();

		void LogInfo(const sgrottel::ISimpleLog& log, bool verbose) const;

	protected:

		// To be called during Ctor to register all parameter object available to the framework
		AbstractCommand& AddParam(const char* name, ParameterBase& param);

		inline const sgrottel::ISimpleLog& Log() const noexcept
		{
			return m_log;
		}

	private:
		const sgrottel::ISimpleLog& m_log;
		std::vector<std::tuple<ParameterBase*, std::string>> m_params;
	};

}
