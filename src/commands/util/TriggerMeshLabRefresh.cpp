#include "TriggerMeshLabRefresh.h"

#include "utilities/CoInitializeScope.h"
#include "utilities/StringUtilities.h"

#include <SimpleLog/SimpleLog.hpp>

#include <filesystem>
#include <string_view>

#include <windows.h>
#include <UIAutomation.h>
#include <oleauto.h>
#include <wrl/client.h>

#pragma comment(lib, "uiautomationcore.lib")

namespace
{
	struct SearchContext
	{
		const sgrottel::ISimpleLog& log;
		std::vector<HWND> windows;
	};

	std::wstring_view BstrView(BSTR b)
	{
		return std::wstring_view(b, SysStringLen(b));
	}
}

using Microsoft::WRL::ComPtr;

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
	utilities::CoInitializeScope coInit;

	auto enumWindowsCallback = [](HWND hwnd, LPARAM lParam) -> BOOL
		{
			SearchContext* ctxt = reinterpret_cast<SearchContext*>(lParam);

			DWORD pid = 0;
			if (GetWindowThreadProcessId(hwnd, &pid) == 0)
			{
				return TRUE;
			}

			if (!IsWindowVisible(hwnd))
			{
				return TRUE;
			}
			if (GetWindow(hwnd, GW_OWNER) != NULL)
			{
				return TRUE;
			}

			HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
			if (h)
			{
				DWORD procNameLen = 2048;
				std::vector<wchar_t> procNameBuf(size_t(procNameLen + 1), 0);
				if (QueryFullProcessImageNameW(h, 0, procNameBuf.data(), &procNameLen) != 0)
				{
					const std::filesystem::path procPath{ std::wstring_view{ procNameBuf.data(), procNameLen } };
					const std::filesystem::path procName{ procPath.filename() };
					if (procName == L"meshlab.exe")
					{
						ctxt->windows.push_back(hwnd);
					}
				}
				CloseHandle(h);
			}

			return TRUE;
		};

	SearchContext ctxt{ .log = Log() };
	EnumWindows(enumWindowsCallback, reinterpret_cast<LPARAM>(&ctxt));

	Log().Detail("Found %d Window(s) matching MeshLab", ctxt.windows.size());

	if (ctxt.windows.empty())
	{
		return true;
	}

	ComPtr<IUIAutomation> automation;
	HRESULT hr = CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&automation));
	if (FAILED(hr))
	{
		throw std::runtime_error(std::format("Failed CoCreateInstance(CLSID_CUIAutomation): {}", hr));
	}

	ComPtr<IUIAutomationTreeWalker> rawWalker;
	hr = automation->get_RawViewWalker(&rawWalker);
	if (FAILED(hr))
	{
		throw std::runtime_error(std::format("Failed automation->get_RawViewWalker: {}", hr));
	}

	//constexpr const std::wstring_view menuItemName = L"Reload All";

	//ComPtr<IUIAutomationCondition> fileMenuCond;
	//{
	//	ComPtr<IUIAutomationCondition> condName;
	//	ComPtr<IUIAutomationCondition> condType;

	//	VARIANT v;
	//	VariantInit(&v);
	//	v.vt = VT_BSTR;
	//	v.bstrVal = SysAllocString(L"File");
	//	hr = automation->CreatePropertyCondition(UIA_NamePropertyId, v, &condName);
	//	VariantClear(&v);
	//	if (FAILED(hr))
	//	{
	//		throw std::runtime_error(std::format("Failed automation->CreatePropertyCondition(name): {}", hr));
	//	}

	//	VariantInit(&v);
	//	v.vt = VT_I4;
	//	v.lVal = UIA_MenuItemControlTypeId;
	//	hr = automation->CreatePropertyCondition(UIA_ControlTypePropertyId, v, &condType);
	//	VariantClear(&v);
	//	if (FAILED(hr))
	//	{
	//		throw std::runtime_error(std::format("Failed automation->CreatePropertyCondition(type): {}", hr));
	//	}

	//	automation->CreateAndCondition(condName.Get(), condType.Get(), &fileMenuCond);
	//}

	//ComPtr<IUIAutomationCacheRequest> cache;
	//hr = automation->CreateCacheRequest(&cache);
	//if (FAILED(hr))
	//{
	//	throw std::runtime_error(std::format("Failed automation->CreateCacheRequest: {}", hr));
	//}

	//{
	//	// Search the RAW view instead of the CONTROL view
	//	cache->put_TreeScope(TreeScope_Subtree);
	//	ComPtr<IUIAutomationCondition> condTrue;
	//	hr = automation->CreateTrueCondition(&condTrue);
	//	if (FAILED(hr))
	//	{
	//		throw std::runtime_error(std::format("Failed automation->CreateTrueCondition: {}", hr));
	//	}
	//	hr = cache->put_TreeFilter(condTrue.Get());
	//	if (FAILED(hr))
	//	{
	//		throw std::runtime_error(std::format("Failed cache->put_TreeFilter(True): {}", hr));
	//	}

	//	// Add properties you want cached
	//	hr = cache->AddProperty(UIA_NamePropertyId);
	//	if (FAILED(hr))
	//	{
	//		throw std::runtime_error(std::format("Failed cache->AddProperty(UIA_NamePropertyId): {}", hr));
	//	}
	//	hr = cache->AddPattern(UIA_InvokePatternId);
	//	if (FAILED(hr))
	//	{
	//		throw std::runtime_error(std::format("Failed cache->AddProperty(UIA_InvokePatternId): {}", hr));
	//	}
	//}

	std::function<ComPtr<IUIAutomationElement>(ComPtr<IUIAutomationElement>, const wchar_t* name, CONTROLTYPEID type)> findInRawTree;

	findInRawTree = [&rawWalker, &log = this->Log(), &findInRawTree](ComPtr<IUIAutomationElement> el, const wchar_t* name, CONTROLTYPEID type) -> ComPtr<IUIAutomationElement>
		{
			ComPtr<IUIAutomationElement> child;
			HRESULT hr = rawWalker->GetFirstChildElement(el.Get(), &child);
			if (FAILED(hr))
			{
				log.Error("Failed to get element children");
				return nullptr;
			}

			while (child)
			{
				bool typeMatch = false;
				if (type != 0)
				{
					CONTROLTYPEID ctrlType;
					hr = child->get_CurrentControlType(&ctrlType);
					if (FAILED(hr))
					{
						log.Error("Failed to get element children control type");
						continue;
					}
					typeMatch = ctrlType == type;
				}
				else
				{
					typeMatch = true;
				}

				bool nameMatch = false;
				{
					BSTR bname = nullptr;
					hr = child->get_CurrentName(&bname);
					if (FAILED(hr))
					{
						log.Error("Failed to get element children name");
						continue;
					}
					nameMatch = (bname && BstrView(bname) == name);
					if (bname)
					{
						SysFreeString(bname);
					}
				}

				if (nameMatch && typeMatch)
				{
					return child;
				}

				// Recurse
				ComPtr<IUIAutomationElement> sub = findInRawTree(child.Get(), name, type);
				if (sub)
				{
					return sub;
				}

				// Next sibling
				ComPtr<IUIAutomationElement> next;
				hr = rawWalker->GetNextSiblingElement(child.Get(), &next);
				if (FAILED(hr))
				{
					log.Error("Failed to get next element child");
					return nullptr;
				}

				child = std::move(next);
			}

			return nullptr;
		};
	auto InvokeElement = [](ComPtr<IUIAutomationElement> element) -> bool
		{
			ComPtr<IUIAutomationInvokePattern> invoke;
			if (SUCCEEDED(element->GetCurrentPatternAs(UIA_InvokePatternId, IID_PPV_ARGS(&invoke))))
			{
				return SUCCEEDED(invoke->Invoke());
			}
			return false;
		};
	auto expandMenuItem = [](ComPtr<IUIAutomationElement> menuItem)
		{
			bool canExpand = false;
			VARIANT v;
			HRESULT hr = menuItem->GetCurrentPropertyValue(UIA_IsExpandCollapsePatternAvailablePropertyId, &v);
			if (SUCCEEDED(hr))
			{
				canExpand = v.boolVal != FALSE;
			}
			VariantClear(&v);

			if (canExpand)
			{
				ComPtr<IUIAutomationExpandCollapsePattern> expand;
				if (SUCCEEDED(menuItem->GetCurrentPatternAs(UIA_ExpandCollapsePatternId, IID_PPV_ARGS(&expand))))
				{
					expand->Expand();
				}

				ComPtr<IUIAutomationInvokePattern> invoke;
				if (SUCCEEDED(menuItem->GetCurrentPatternAs(UIA_InvokePatternId, IID_PPV_ARGS(&invoke))))
				{
					invoke->Invoke();
				}
			}
		};
	auto collapseMenuItem = [](ComPtr<IUIAutomationElement> menuItem)
		{
			bool canCollapse = false;
			VARIANT v;
			HRESULT hr = menuItem->GetCurrentPropertyValue(UIA_IsExpandCollapsePatternAvailablePropertyId, &v);
			if (SUCCEEDED(hr))
			{
				canCollapse = v.boolVal != FALSE;
			}
			VariantClear(&v);

			if (canCollapse)
			{
				ComPtr<IUIAutomationExpandCollapsePattern> collapse;
				if (SUCCEEDED(menuItem->GetCurrentPatternAs(UIA_ExpandCollapsePatternId, IID_PPV_ARGS(&collapse))))
				{
					collapse->Collapse();
				}

				ComPtr<IUIAutomationInvokePattern> invoke;
				if (SUCCEEDED(menuItem->GetCurrentPatternAs(UIA_InvokePatternId, IID_PPV_ARGS(&invoke))))
				{
					invoke->Invoke();
				}
			}
		};

	for (HWND hwnd : ctxt.windows)
	{
		ComPtr<IUIAutomationElement> wndRoot;
		hr = automation->ElementFromHandle(hwnd, &wndRoot);
		if (FAILED(hr))
		{
			Log().Warning("Window 0x%.08x has no automation root: %d", reinterpret_cast<int>(hwnd), hr);
			continue;
		}

		ComPtr<IUIAutomationElement> reloadAll = findInRawTree(wndRoot, L"Reload All", UIA_ButtonControlTypeId);
		if (!reloadAll)
		{
			Log().Detail("Window without 'Reload All' toolbar button");
		}
		else
		{
			if (InvokeElement(reloadAll))
			{
				Log().Detail("MeshLab [0x%.08x] \"Reload All\" toolbar button triggered", reinterpret_cast<uintptr_t>(hwnd));
				continue;
			}
		}

		ComPtr<IUIAutomationElement> fileMenu = findInRawTree(wndRoot, L"File", UIA_MenuItemControlTypeId);
		if (!fileMenu)
		{
			Log().Detail("Window without 'File' menu item");
		}
		else
		{
			expandMenuItem(fileMenu);
			reloadAll = findInRawTree(fileMenu, L"Reload All", UIA_MenuItemControlTypeId);
			collapseMenuItem(fileMenu);
			if (!reloadAll)
			{
				Log().Detail("Window without 'File | Reload All' menu item");
			}
			else
			{
				if (InvokeElement(reloadAll))
				{
					Log().Detail("MeshLab [0x%.08x] \"Reload All\" menu item triggered", reinterpret_cast<uintptr_t>(hwnd));
					continue;
				}
			}
		}
	}

	return true;
}
