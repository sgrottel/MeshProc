#include "SvgShape2DWriter.h"

#include <SimpleLog/SimpleLog.hpp>

#include <tinyxml2/tinyxml2.h>

#include <algorithm>

using namespace meshproc;
using namespace meshproc::io;

SvgShape2DWriter::SvgShape2DWriter(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::String>("Path", m_path);
	AddParamBinding<ParamMode::In, ParamType::Shape2D>("Shape", m_shape);
}

bool SvgShape2DWriter::Invoke()
{
	using namespace tinyxml2;

	if (m_path.empty())
	{
		Log().Error("Path not set");
		return false;
	}
	if (!m_shape)
	{
		Log().Error("Shape not set");
		return false;
	}

	::tinyxml2::XMLDocument doc;
	XMLElement* root = doc.NewElement("svg");
	doc.InsertFirstChild(root);
	root->SetAttribute("xmlns", "http://www.w3.org/2000/svg");

	float minX = 0.0f, minY = 0.0f, maxX = 0.0f, maxY = 0.0f;
	{
		bool first = true;
		for (auto const& loop : m_shape->loops)
		{
			if (loop.second.empty()) continue;
			if (first)
			{
				first = false;
				minX = maxX = loop.second.front().x;
				minY = maxY = loop.second.front().y;
			}
			for (glm::vec2 const& p : loop.second)
			{
				if (minX > p.x) minX = p.x;
				if (maxX < p.x) maxX = p.x;
				if (minY > p.y) minY = p.y;
				if (maxY < p.y) maxY = p.y;
			}
		}
	}

	if (maxX - minX <= 0.0f) maxX = minX + 100.0f;
	if (maxY - minY <= 0.0f) maxY = minY + 100.0f;

	root->SetAttribute("width", maxX - minX);
	root->SetAttribute("height", maxY - minY);
	root->SetAttribute("viewBox", std::format("{} {} {} {}", minX, minY, maxX, maxY).c_str());

	for (auto const& loop : m_shape->loops)
	{
		if (loop.second.empty()) continue;

		XMLElement* path = doc.NewElement("path");
		root->InsertEndChild(path);

		std::stringstream p;
		auto it = loop.second.begin();
		p << 'M' << it->x << ' ' << it->y;
		for (++it; it != loop.second.end(); ++it)
		{
			p << " L" << it->x << ' ' << it->y;
		}
		p << " Z";

		path->SetAttribute("d", p.str().c_str());
		path->SetAttribute("fill", "none");
		path->SetAttribute("stroke", "black");
		path->SetAttribute("stroke-width", "1");
	}


	FILE* file = nullptr;
	errno_t r = _wfopen_s(&file, m_path.c_str(), L"wb");
	if (r != 0) {
		wchar_t errMsg[95]{};
		_wcserror_s(errMsg, r);
		Log().Error(L"Failed to open \"%s\": %s (%d)", m_path.c_str(), errMsg, static_cast<int>(r));
		return false;
	}
	if (file == nullptr) {
		Log().Error(L"Failed to open \"%s\": returned nullptr", m_path.c_str());
		return false;
	}

	XMLError eResult = doc.SaveFile(file);
	fclose(file);
	if (eResult != XML_SUCCESS) {
		Log().Error(L"Error saving XML: %d\n", eResult);
		return eResult;
	}

	/*
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
*/
	return true;
}
