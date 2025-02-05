#pragma once

#include "Mesh.h"

#include <string>
#include <memory>

namespace sgrottel
{
	class ISimpleLog;
}

class StlWriter
{
public:
	StlWriter(sgrottel::ISimpleLog& log);

	void Save(std::wstring const& filename, std::shared_ptr<Mesh> mesh);

private:
	sgrottel::ISimpleLog& m_log;
};

