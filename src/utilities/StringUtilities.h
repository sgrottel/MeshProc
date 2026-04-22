#pragma once

#include <string>
#include <string_view>

namespace meshproc
{
	std::string ToUtf8(const std::wstring& wstr);
	std::string ToUtf8(const std::wstring_view& wstr);
	std::wstring FromUtf8(const std::string& str);
}
