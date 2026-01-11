#pragma once

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{
	namespace commands
	{
		class CommandFactory;

		bool CommandRegistration(class CommandFactory& factory, const sgrottel::ISimpleLog& log);

	}
}
