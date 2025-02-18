#include "AbstractCommand.h"

using namespace meshproc;

AbstractCommand::AbstractCommand(const sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}
