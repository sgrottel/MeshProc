#pragma once

#include "AbstractCommand.h"
#include "data/Scene.h"

#include <filesystem>
#include <memory>

namespace meshproc
{
	namespace io
	{

		class PlyReader : public AbstractCommand
		{
		public:
			PlyReader(const sgrottel::ISimpleLog& log);

			bool Invoke() override;

		private:
			const std::wstring m_path{};
			std::shared_ptr<data::Mesh> m_mesh{};
		};

	}
}
