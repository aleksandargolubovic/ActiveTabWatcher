#pragma once
#include <oleacc.h>

namespace Watcher
{
	class ActiveTabWatcher
	{
	public:
		ActiveTabWatcher();
		~ActiveTabWatcher();

	private:
		static void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
		static HRESULT CheckName(IAccessible * pAcc, long childId);
		static HRESULT WalkTreeWithAccessibleChildren(IAccessible * pAcc);

		HWINEVENTHOOK m_hook;
	};

}