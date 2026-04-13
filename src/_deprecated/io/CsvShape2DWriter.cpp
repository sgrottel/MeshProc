#include "CsvShape2DWriter.h"

#include <SimpleLog/SimpleLog.hpp>

#include <fstream>

using namespace meshproc;
using namespace meshproc::io;

CsvShape2DWriter::CsvShape2DWriter(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::In, ParamType::Shape2D>("Shape", m_shape);
}

bool CsvShape2DWriter::Invoke()
{
	std::fstream file(m_path, std::ios_base::out | std::ios::trunc | std::ios::binary);
	if (!file.is_open())
	{
		Log().Error(L"Failed to open \"%s\"", m_path);
		return false;
	}
	if (!m_shape)
	{
		Log().Error("Shape not set");
		return false;
	}

	file << "x;y;loopStartId\n" << std::setprecision(std::numeric_limits<float>::max_digits10);

	for (const auto& p : m_shape->loops)
	{
		for (size_t i = 0; i < p.second.size(); ++i)
		{
			const auto& v = p.second[i];
			file << v.x << ";" << v.y << ";";
			if (i == 0)
			{
				file << p.first;
			}
			file << "\n";
		}
	}

	file.close();

	return true;
}
