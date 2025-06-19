#include "StringUtilities.h"

#include <windows.h>

std::string meshproc::ToUtf8(const std::wstring& wstr)
{
	if (wstr.empty())
	{
		return {};
	}

	int size_needed = WideCharToMultiByte(
		CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr
	);

	std::string result(size_needed - 1, 0); // exclude null terminator
	WideCharToMultiByte(
		CP_UTF8, 0, wstr.c_str(), -1, result.data(), size_needed, nullptr, nullptr
	);

	return result;
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
