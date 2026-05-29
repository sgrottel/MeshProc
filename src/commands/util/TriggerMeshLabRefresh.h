#pragma once

#include "commands/AbstractCommand.h"

namespace meshproc
{
	namespace commands
	{
		namespace util
		{
			class TriggerMeshLabRefresh : public AbstractCommand
			{
			public:
				TriggerMeshLabRefresh(const sgrottel::ISimpleLog& log);
				bool Invoke() override;

			private:
				const std::wstring m_path{};
			};
		}
	}
}
