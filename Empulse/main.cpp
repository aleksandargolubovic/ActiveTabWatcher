#include "ActiveTabWatcher.h"

int main(int argc, const char* argv[]) 
{
	MSG msg;
	Watcher::ActiveTabWatcher chromeWatcher;

	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}