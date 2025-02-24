#pragma once

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{
	class AbstractCommand;

	class CommandFactory
	{
	public:
		CommandFactory(const sgrottel::ISimpleLog& log);

		template<typename T>
		bool Register(const char* name)
		{
			// TODO: Implement
			return true;
		}

	private:
		const sgrottel::ISimpleLog& m_log;
	};

}
