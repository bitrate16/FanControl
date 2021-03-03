#include <Windows.h>
#include <Winuser.h>
#include <Ioapiset.h>
#include <winioctl.h>
#include <cstdlib>
#include <cstdio>
#include <Tchar.h>

DWORD write_fast() {
	HANDLE hndl = CreateFileW(L"\\\\.\\EnergyDrv", 0xC0000000, 0, 0i64, 2u, 0x80u, 0i64);
	if (hndl == (HANDLE)-1i64) {
		printf("Failed open %s", "\\\\.\\EnergyDrv");
		return -1;
	}

	// lpInBuffer value: 06 00 00 00  01 00 00 00  01 00 00 00 ~ [ 6, 1, 1 ] (inv endian)
	DWORD in[3];
	in[0] = 6;
	in[1] = 1;
	in[2] = 1;
		
	DWORD v16 = 0;

	DeviceIoControl(hndl, 0x831020C0, in, 12u, NULL, 0, &v16, 0);
	CloseHandle(hndl);

	return 0;
}

DWORD write_normal() {
	HANDLE hndl = CreateFileW(L"\\\\.\\EnergyDrv", 0xC0000000, 0, 0i64, 2u, 0x80u, 0i64);
	if (hndl == (HANDLE)-1i64) {
		printf("Failed open %s", "\\\\.\\EnergyDrv");
		return -1;
	}

	// lpInBuffer value: 06 00 00 00  01 00 00 00  00 00 00 00 ~ [ 6, 1, 1 ] (inv endian)
	DWORD in[3];
	in[0] = 6;
	in[1] = 1;
	in[2] = 0;
		
	DWORD v16 = 0;

	DeviceIoControl(hndl, 0x831020C0, in, 12u, NULL, 0, &v16, 0);
	CloseHandle(hndl);

	return 0;
}

DWORD read_state() {
	HANDLE hndl = CreateFileW(L"\\\\.\\EnergyDrv", 0xC0000000, 0, 0i64, 2u, 0x80u, 0i64);
	if (hndl == (HANDLE)-1i64) {
		printf("Failed open %s", "\\\\.\\EnergyDrv");
		return -1;
	}
		
	// lpInBuffer value: 0E 00 00 00 ~ [ 14 ] (inv endian)
	DWORD in = 14;
		
	DWORD out;
		
	DWORD v16 = 0;

	DeviceIoControl(hndl, 0x831020C4, &in, 4u, &out, 4u, &v16, 0);
	CloseHandle(hndl);

	return out;
}

// Used to send "normal" speed state to fan on program exit
void exit_handler() {
	write_normal();
}

BOOL WINAPI ConsoleHandler(DWORD dwType) {
	switch(dwType) {
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:

			write_normal();

			Sleep(1000);

			// Sometimes it does not stop
			write_normal();

			Sleep(100);

			ExitProcess(0);

			return TRUE;
		default:
			break;
	}
	return FALSE;
}

void print_help() {
	printf("Use FanControl.exe \"help\" for help\n");
	printf("Use FanControl.exe \"fast\" for 100%% speed\n");
	printf("Use FanControl.exe \"normal\" for basic speed");
	printf("Use FanControl.exe \"read\" for get CleanDustData (driver response code)\n");
	printf("Use FanControl.exe \"holdfast\" \"[value]\" to automatically request 100%% speed when driver responds with code different from <value> (example: FanControl.exe holdfast 3)\n");
}

// https://stackoverflow.com/questions/38971064/c-event-handling-for-click-on-notification-area-icon-windows
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    static NOTIFYICONDATA nid;

    switch (iMsg)
    {
        case WM_CREATE:
            memset(&nid, 0, sizeof(nid));
            nid.cbSize = sizeof(nid);
            nid.hWnd = hWnd;
            nid.uID = 0;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            nid.uCallbackMessage = WM_APP + 1;
            nid.hIcon = LoadIcon(nullptr, IDI_WARNING);
			lstrcpy(nid.szTip, (LPTSTR) L"FanControl (right click to exit)");
            Shell_NotifyIcon(NIM_ADD, &nid);
            Shell_NotifyIcon(NIM_SETVERSION, &nid);
            return 0;
        case WM_APP + 1:
            switch (lParam) {
                case WM_RBUTTONUP:
					PostQuitMessage(0);
                    break;
            }
            break;
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd,iMsg,wParam,lParam);
}

int main(int argc, const char** argv) {

	if (argc == 1) {

		// Hide console, show tray icon with right click exit
		::ShowWindow(::GetConsoleWindow(), SW_HIDE);

		LPCWSTR lpszClass = L"__hidden__";

		HINSTANCE hInstance = GetModuleHandle(nullptr);

		WNDCLASS wc;
		HWND hWnd;
		MSG msg;

		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hbrBackground = nullptr;
		wc.hCursor = nullptr;
		wc.hIcon = nullptr;
		wc.hInstance = hInstance;
		wc.lpfnWndProc = WndProc;
		wc.lpszClassName = lpszClass;
		wc.lpszMenuName = nullptr;
		wc.style = 0;
		RegisterClass(&wc);

		hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			nullptr, nullptr, hInstance, nullptr);

		// Hardcoded from my driver version
		int expected_value = 3;

		std::atexit(exit_handler);
		SetConsoleCtrlHandler(ConsoleHandler, true);

		while (true) {
			int state = read_state();

			if (state == -1)
				return 0;

			if (state != expected_value) 
				if (write_fast() == -1)
					return 0;
			
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (msg.message == WM_QUIT) {

				write_normal();

				Sleep(1000);

				// Sometimes it does not stop
				write_normal();

				Sleep(100);

				ExitProcess(0);
			}
			
			// Sleep for 2 seconds because fan slowdown timeout window is 2 seconds
			Sleep(2000);
		}

		return static_cast<int>(msg.wParam);
		
	} else if (strcmp(argv[1], "help") == 0) {

		print_help();

	} else if (strcmp(argv[1], "fast") == 0) {

		write_fast();

	} else if (strcmp(argv[1], "normal") == 0) {

		write_normal();

	} else if (strcmp(argv[1], "read") == 0) {

		printf("CleanDustData: %d", read_state());

	} else if (strcmp(argv[1], "holdfast") == 0) {

		// Hardcoded from my driver version
		int expected_value = 3;
		
		if (argc > 2)
			expected_value = atoi(argv[2]);

		std::atexit(exit_handler);
		SetConsoleCtrlHandler(ConsoleHandler, true);

		while (true) {
			int state = read_state();

			if (state == -1)
				return 0;

			if (state != expected_value) 
				if (write_fast() == -1)
					return 0;
			
			// Sleep for 2 seconds because fan slowdown timeout window is 2 seconds
			Sleep(2000);
		}

	} else {
		
		print_help();

	}

	return 0;
}