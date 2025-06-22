#pragma once

#include <string>

namespace meshproc
{
	std::string ToUtf8(const std::wstring& wstr);
	std::wstring FromUtf8(const std::string& str);
}
