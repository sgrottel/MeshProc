#pragma once

#include "Parameter.h"

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

		void PreInvoke();
		virtual bool Invoke() = 0;
		void PostInvoke();

		void LogInfo(const sgrottel::ISimpleLog& log, bool verbose) const;

		// Framework function, only to be called by `MeshProgram`
		ParameterBase* AccessParam(const std::string& name) const;

	protected:

		// To be called during Ctor to register all parameter object available to the framework
		AbstractCommand& AddParam(const std::string& name, ParameterBase& param);

		inline const sgrottel::ISimpleLog& Log() const noexcept
		{
			return m_log;
		}

	private:
		const sgrottel::ISimpleLog& m_log;
		std::unordered_map<std::string, ParameterBase*> m_params;
	};

}
