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

			inline void SetPath(const std::wstring& path)
			{
				const_cast<std::wstring&>(m_path) = path;
			}

			inline std::shared_ptr<data::Mesh> GetMesh() const
			{
				return m_mesh;
			}

		private:
			const std::wstring m_path{};
			std::shared_ptr<data::Mesh> m_mesh{};
		};

	}
}
