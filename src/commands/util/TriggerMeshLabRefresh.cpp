#include "TriggerMeshLabRefresh.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::util;

TriggerMeshLabRefresh::TriggerMeshLabRefresh(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
}

bool TriggerMeshLabRefresh::Invoke()
{



	return true;
}
