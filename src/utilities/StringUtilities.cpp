#include "StringUtilities.h"

#include <windows.h>

namespace
{
	inline std::string ToUtf8Impl(const wchar_t* wstr, size_t wstrSize)
	{
		if (wstr == nullptr || wstrSize <= 0)
		{
			return {};
		}

		int size_needed = WideCharToMultiByte(
			CP_UTF8, 0, wstr, static_cast<int>(wstrSize), nullptr, 0, nullptr, nullptr
		);

		std::string result(size_needed - 1, 0); // exclude null terminator
		WideCharToMultiByte(
			CP_UTF8, 0, wstr, static_cast<int>(wstrSize), result.data(), size_needed, nullptr, nullptr
		);

		return result;
	}
}

std::string meshproc::ToUtf8(const std::wstring& wstr)
{
	return ToUtf8Impl(wstr.c_str(), wstr.size());
}

std::string meshproc::ToUtf8(const std::wstring_view& wstr)
{
	return ToUtf8Impl(wstr.data(), wstr.size());
}

std::wstring meshproc::FromUtf8(const std::string& str)
{
	if (str.empty())
	{
		return {};
	}

	int size_needed = MultiByteToWideChar(
		CP_UTF8, 0, str.c_str(), -1, nullptr, 0
	);

	std::wstring result(size_needed - 1, 0); // exclude null terminator
	MultiByteToWideChar(
		CP_UTF8, 0, str.c_str(), -1, result.data(), size_needed
	);

	return result;
}
