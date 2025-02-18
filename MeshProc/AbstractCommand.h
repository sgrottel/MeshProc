#pragma once

#include "Parameter.h"

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

	protected:

		inline const sgrottel::ISimpleLog& Log() const noexcept
		{
			return m_log;
		}

	private:
		const sgrottel::ISimpleLog& m_log;
	};

}
