#include "Instance.h"

void CheckInstance(void) {
	// Check if another instance of the application is already running
	HANDLE hMutex = CreateMutex(NULL, FALSE, __MUTEX_NAME__);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// Another instance is running, so ask the user what to do
		CloseHandle(hMutex);
		int result = MessageBox(NULL, L"Another instance of XPClock is already running. It may be in your system tray or in the background processes.\r\n\r\nIf you would like to run a new instance of the application and exit the one that is already running, click yes.\r\nIf you would like to bring the instance already running to the foreground, click no.\r\nIf you would like to exit this instance and do nothing, click cancel.", L"Instance Check", MB_YESNOCANCEL | MB_ICONWARNING);
		switch (result) {
		case IDYES: {
			// Terminate the existing instance
			HWND hWnd = FindWindow(NULL, L"XPClock");
			if (hWnd) {
				PostMessage(hWnd, WM_CLOSE, 0, 0);
			}
			// Create a new instance
			HANDLE hNewMutex = CreateMutex(NULL, FALSE, __MUTEX_NAME__);
			if (!hNewMutex) {
				MessageBox(NULL, L"Failed to create new mutex! The application will now exit.", L"Error", MB_OK | MB_ICONERROR);
				exit(GetLastError());
			}
			break;
		}
		case IDNO: {
			// Bring the existing instance to the foreground
			HWND hWnd = FindWindow(NULL, L"XPClock");
			if (hWnd) {
				SetForegroundWindow(hWnd);
			}
			exit(0);
			break;
		}
		case IDCANCEL:
			// Exit this instance
			CloseHandle(hMutex);
			exit(0);
			return;
		}
	}
}