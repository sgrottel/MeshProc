#include "CsvShape2DReader.h"

#include <SimpleLog/SimpleLog.hpp>

#include <fstream>
#include <regex>
#include <string>

using namespace meshproc;
using namespace meshproc::io;

CsvShape2DReader::CsvShape2DReader(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::Out, ParamType::Shape2D>("Shape", m_shape);
}

bool CsvShape2DReader::Invoke()
{
	std::ifstream file{ m_path, std::ios::in };
	if (!file.is_open())
	{
		Log().Error("Failed to open file");
		return false;
	}

	Log().Message(L"Reading CSV: %s", m_path.c_str());

	std::string lineBuf;
	if (!std::getline(file, lineBuf))
	{
		Log().Error("Failed to read first line");
		return false;
	}

	std::regex headerLine(R"(^\s*x\s*;\s*y\s*;\s*loopStartId\s*$)");
	std::smatch matches;

	if (!std::regex_match(lineBuf, matches, headerLine))
	{
		Log().Error("Unexpected header line format; must be exactly as output by CsvShape2DWriter");
		return false;
	}

	std::regex dataLine(R"(^\s*(-?[\.\d]+)\s*;\s*(-?[\.\d]+)\s*;\s*(-?[\d]*)\s*$)");

	auto shape = std::make_shared<data::Shape2D>();

	size_t lineId = 0;
	size_t lineCnt = 1;
	while (std::getline(file, lineBuf))
	{
		lineCnt++;
		if (lineBuf.empty()) continue;
		if (lineBuf.front() == '#') continue;

		if (!std::regex_match(lineBuf, matches, dataLine))
		{
			Log().Error("Failed parsing data line %d", lineCnt);
			return false;
		}

		double x = std::atof(matches[1].str().c_str());
		double y = std::atof(matches[2].str().c_str());
		if (matches[3].length() > 0)
		{
			long id = std::atol(matches[3].str().c_str());
			lineId = id;
		}

		shape->loops[lineId].push_back({ x, y });
	}

	m_shape = shape;
	return true;
}
