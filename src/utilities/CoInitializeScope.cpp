#include "CoInitializeScope.h"

#include <format>
#include <mutex>

#include <Objbase.h>

#pragma comment(lib, "ole32.lib")

namespace
{
	static std::mutex g_coInitMutex{};
	static int g_coInitCounter = 0;
}

meshproc::utilities::CoInitializeScope::CoInitializeScope()
{
	std::scoped_lock<std::mutex> lock{ g_coInitMutex };
	if (g_coInitCounter == 0)
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (hr != S_OK)
		{
			throw std::runtime_error(std::format("Failed to CoInitializeEx: {}", hr));
		}
	}
	g_coInitCounter++;
}

meshproc::utilities::CoInitializeScope::~CoInitializeScope()
{
	std::scoped_lock<std::mutex> lock{ g_coInitMutex };
	g_coInitCounter--;
	if (g_coInitCounter == 0)
	{
		CoUninitialize();
	}
}
