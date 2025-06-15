#pragma once

#include "AbstractCommand.h"
#include "data/Scene.h"

#include <filesystem>
#include <memory>

namespace meshproc
{
	namespace io
	{

		class PlyWriter : public AbstractCommand
		{
		public:
			PlyWriter(const sgrottel::ISimpleLog& log);

			bool Invoke() override;

			inline void SetPath(const std::wstring& path)
			{
				const_cast<std::wstring&>(m_path) = path;
			}

			inline void SetScene(std::shared_ptr<data::Scene>& scene)
			{
				const_cast<std::shared_ptr<data::Scene>&>(m_scene) = scene;
			}

		private:
			const std::wstring m_path{};
			const std::shared_ptr<data::Scene> m_scene{};
		};

	}
}
