#include "Clock.h"
#include "Instance.h"
#include "Colors.h"
#include "NTPClient.h"
#include "AboutWindow.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance); // https://learn.microsoft.com/en-us/archive/msdn-magazine/2005/may/c-at-work-unreferenced-parameters-adding-task-bar-commands
	UNREFERENCED_PARAMETER(lpCmdLine);

	CheckInstance(); // Check if another instance of the application is already running

	CreateReg(); // Fixed a bug where the application would crash if the registry key didn't exist.

	if (ConsoleEnabled()) {
		// If the user wants the console to be open, open the console.
		AllocConsole();

		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		// Don't need stdin for anything, so don't open it.
	}

	// See colors.h for this call and like calls.
	green();
	wprintf(L"Welcome to XPClock! It appears you have the console logging option turned on.\r\nThis will output debug messages whenever specific functions are called.\r\n\r\n");
	reset();

	// https://learn.microsoft.com/en-us/windows/win32/controls/cc-faq
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_WIN95_CLASSES;
	if (!InitCommonControlsEx(&icex)) {
		red();
		wprintf(L"Error initializing common controls! Execution cannot proceed.\r\n"); // Print to the console that common controls fails to initialize.
		reset();
		FormattedMessageBox(NULL, L"Common Controls initialization failed!\r\n\r\nError code: 0x%x\r\n\r\nExecution cannot continue. Please report this error to the XPClock github page.", L"XPClock", MB_OK | MB_ICONERROR, GetLastError());
		return GetLastError();
	}

	// Register the window class and show the window.
	wprintf(L"Registered main class.\r\n");

	// Intitialize the instance of the application
	if (!InitInstance(hInstance, nCmdShow)) {
		red();
		wprintf(L"\r\nError initializing the instance of the application! This most likely indicates that the window failed to create. Please check the message box.\r\n");
		reset();
		FormattedMessageBox(NULL, L"Instance initialization failed!\r\nError code: 0x%x\r\n\r\nThe application will now exit.", L"Severe Error!", MB_OK | MB_ICONERROR, GetLastError());
		return GetLastError();
	}

	// Main message loop. https://learn.microsoft.com/en-us/windows/win32/winmsg/using-messages-and-message-queues
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
			switch (g_Context) {
			case __CONTEXT_CLOCK__:
				wprintf(L"Quiting application...\r\n");
				PostQuitMessage(0);
				break;
			case __CONTEXT_ABOUT__:
				wprintf(L"Escape key recieved. Destroying about window.\r\n");
				DestroyWindow(g_hWndAbout);
				break;
			case __CONTEXT_SETTINGS__:
				wprintf(L"Escape key recieved. Destroying settings window.\r\n");
				DestroyWindow(g_hWndSettings);
				break;
			}
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	blue();
	wprintf(L"Initializing instance...\r\n\r\n");
	reset();

	g_hInst = hInstance;

	g_szMainFont = GetCustomFont();
	wprintf(L"Setting up fonts with font name: '%s' \r\n", g_szMainFont);

	// Create the fonts
	g_hfMainFont = CreateFont(48, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, g_szMainFont);
	g_hfBtnFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, g_szMainFont);

	// Set config structure values to prevent repeated system calls
	wprintf(L"Parsing configuration from registry...\r\n");
	g_Config.CustomColor = CustomColor();
	g_Config.DVDLogo = DVDLogo();
	g_Config.Gradient = GradientUsed();
	g_Config.DisplayFormat = GetDisplayFormat();
	g_Config.ConsoleEnabled = ConsoleEnabled();
	g_Config.TrayIconEnabled = TrayIconEnabled();
	g_Config.MenuEnabled = MenuEnabled();

	if (g_Config.MenuEnabled) {
		g_hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
	}
	else {
		g_hMenu = NULL;
	}

	wprintf(L"g_Config (%p) fields successfully parsed with values\r\nCustomColor: %d\r\nConsoleEnabled: %d\r\nDisplayFormat: %d\r\nDVDLogo: %d\r\nGradient: %d\r\nTrayIconEnabled: %d\r\nMenuEnabled: %d\r\n", &g_Config, g_Config.CustomColor, g_Config.ConsoleEnabled, g_Config.DisplayFormat, g_Config.DVDLogo, g_Config.Gradient, g_Config.TrayIconEnabled, g_Config.MenuEnabled);

	if (g_Config.TrayIconEnabled) {
		wprintf(L"Starting tray icon...\r\n");
		StartTray();
	}

	wprintf(L"Parsing time config...\r\n");
	GetTimeConfig(&g_TimeConfig);
	g_nTimeZone = GetTimeZone();
	wprintf(L"g_TimeConfig (%p) parsed successfully.\r\n", &g_TimeConfig);

	// Parses the clock.col file if the setting is set to true
	if (g_Config.CustomColor) {
		wprintf(L"Parsing custom color from '%s'\r\n", GetColorFile());
		ParseCustomColor();
	}

	if (g_TimeConfig.ts == 1) {
		wprintf(L"Creating NTP sync thread.\r\n");
		g_hNTPThread = CreateThread(NULL, 0, NTPThread, NULL, 0, &g_tidNTPThread);
		if (!g_hNTPThread) {
			red();
			wprintf(L"Failed to create NTP sync thread! GetLastError: 0x%x\r\n", GetLastError());
			reset();
			MessageBox(NULL, L"Failed to create NTP sync thread!", L"Error", MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}

	// Create the main window and show it.
	if (!CreateClock()) {
		return FALSE;
	}

	ShowWindow(g_hWndMain, nCmdShow);
	UpdateWindow(g_hWndMain);

	return TRUE;
}