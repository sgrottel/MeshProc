#pragma once

#include "AbstractCommand.h"
#include "data/Shape2D.h"

#include <filesystem>
#include <memory>

namespace meshproc
{
	namespace io
	{

		class CsvShape2DReader : public AbstractCommand
		{
		public:
			CsvShape2DReader(const sgrottel::ISimpleLog& log);

			bool Invoke() override;

		private:
			const std::wstring m_path{};
			std::shared_ptr<data::Shape2D> m_shape{};
		};

	}
}
