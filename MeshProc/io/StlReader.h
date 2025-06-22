#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <filesystem>

namespace meshproc
{
	namespace io
	{

		class StlReader : public AbstractCommand
		{
		public:
			StlReader(const sgrottel::ISimpleLog& log);

			bool Invoke() override;

		private:
			const std::wstring m_path{};
			std::shared_ptr<data::Mesh> m_mesh{};
		};

	}
}
