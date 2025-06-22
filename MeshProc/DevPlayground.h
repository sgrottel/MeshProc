#pragma once

#include "AbstractCommand.h"

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{
	class DevPlayground : public AbstractCommand
	{
	public:
		DevPlayground(const sgrottel::ISimpleLog& log);
		bool Invoke() override;
	private:
	};
}
