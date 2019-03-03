#include "ActiveTabWatcher.h"
#include <iostream>
#include <Windows.h>

namespace Watcher
{
	const std::string CHROME_WIDGET = "Chrome_WidgetWin_1";
	const std::wstring ADDRESS_BAR = L"Address and search bar";

	ActiveTabWatcher::ActiveTabWatcher() : m_hook{nullptr}
	{
		CoInitialize(NULL);
		m_hook = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_VALUECHANGE, 0, WinEventProc, 0, 0, WINEVENT_SKIPOWNPROCESS);
	}

	ActiveTabWatcher::~ActiveTabWatcher()
	{
		if (m_hook == nullptr)
			return;
		UnhookWinEvent(m_hook);
		CoUninitialize();
	}

	HRESULT ActiveTabWatcher::CheckName(IAccessible* pAcc, long childId)
	{
		if (pAcc == nullptr)
			return E_INVALIDARG;
		BSTR bstrName;
		VARIANT varChild;
		varChild.vt = VT_I4;
		varChild.lVal = childId;
		HRESULT ret = S_FALSE;
		if (pAcc->get_accName(varChild, &bstrName) == S_OK && wcscmp(bstrName, ADDRESS_BAR.c_str()) == 0)
		{
			BSTR bstrValue = nullptr;
			if (pAcc->get_accValue(varChild, &bstrValue) == S_OK && wcscmp(bstrValue, L"") != 0)
			{
				ret = S_OK;
				printf("----------------------------------\n");
				printf("EVENT_OBJECT_FOCUS %ls\n", bstrValue);
				SysFreeString(bstrValue);
			}
		}
		SysFreeString(bstrName);
		return ret;
	}

	HRESULT ActiveTabWatcher::WalkTreeWithAccessibleChildren(IAccessible* pAcc)
	{
		if (pAcc == nullptr)
			return E_INVALIDARG;
		HRESULT hr;
		long childCount;
		long returnCount;

		hr = pAcc->get_accChildCount(&childCount);
		if (FAILED(hr))
			return hr;
		if (childCount == 0)
			return S_FALSE;
		VARIANT* pArray = new VARIANT[childCount];
		hr = AccessibleChildren(pAcc, 0L, childCount, pArray, &returnCount);
		if (FAILED(hr))
			return hr;
		//--- Iterate through children.
		for (int x = 0; x < returnCount; x++)
		{
			VARIANT vtChild = pArray[x];
			//--- If it's an accessible object, get the IAccessible, and recurse.
			if (vtChild.vt == VT_DISPATCH)
			{
				IDispatch* pDisp = vtChild.pdispVal;
				IAccessible* pChild = nullptr;
				if (pDisp->QueryInterface(IID_IAccessible, (void**)&pChild) == S_OK)
				{
					hr = CheckName(pChild, CHILDID_SELF);
					if (hr != S_OK)
						hr = WalkTreeWithAccessibleChildren(pChild);
					pChild->Release();

				}
				pDisp->Release();
				//--- break the for loop if url was found
				if (hr == S_OK)
					break;
			}
		}
		delete[] pArray;
		return hr;
	}

	void CALLBACK ActiveTabWatcher::WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
	{
		DWORD tsFunctionEnter = GetTickCount();
		DWORD tsFoundUrl = 0;
		if (event != EVENT_OBJECT_FOCUS && event != EVENT_OBJECT_VALUECHANGE)
			return;
		
		IAccessible* pAcc = nullptr;
		VARIANT varChild;
		if ((AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild) == S_OK) && (pAcc != nullptr))
		{
			char className[50];
			if (GetClassName(hwnd, className, 50))
			{
				if (strcmp(className, CHROME_WIDGET.c_str()) == 0)
				{
					BSTR bstrName = nullptr;
					if (pAcc->get_accName(varChild, &bstrName) == S_OK && ADDRESS_BAR.compare(bstrName) == 0)
					{
						//--- early exit
						BSTR bstrValue = nullptr;
						if (pAcc->get_accValue(varChild, &bstrValue) == S_OK)
						{
							tsFoundUrl = GetTickCount();
							printf("----------------------------------\n");
							printf("EVENT_OBJECT_VALUECHANGE %ls\n", bstrValue);
							printf("dt1=%ld dt2=%ld\n", tsFunctionEnter - dwmsEventTime, tsFoundUrl - tsFunctionEnter);
							SysFreeString(bstrValue);
						}
						SysFreeString(bstrName);

					}
					else if (WalkTreeWithAccessibleChildren(pAcc) == S_OK)
					{
						tsFoundUrl = GetTickCount();
						printf("dt1=%ld dt2=%ld\n", tsFunctionEnter - dwmsEventTime, tsFoundUrl - tsFunctionEnter);
					}
					pAcc->Release();
				}
			}
		}
	}
}