#pragma once

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{
	class CommandFactory;

	bool CommandRegistration(class CommandFactory& factory, const sgrottel::ISimpleLog& log);

}

