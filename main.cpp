#include <shlobj.h>
#include <windows.h>

void ToggleArrows(bool remove, HWND hWnd) {
    HKEY hKey;
    LSTATUS status = RegCreateKeyExA(
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Icons", 0,
        NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL
    );

    bool success = false;
    if (status == ERROR_SUCCESS) {
        if (remove) {
            const char *iconPath = "shell32.dll,50";
            status = RegSetValueExA(hKey, "29", 0, REG_SZ, (const BYTE *)iconPath, lstrlenA(iconPath) + 1);
            if (status == ERROR_SUCCESS) {
                success = true;
            } else {
                char buf[100];
                wsprintfA(buf, "RegSetValueExA failed with error: %d", status);
                MessageBoxA(hWnd, buf, "Error", MB_ICONERROR);
            }
        } else {
            status = RegDeleteValueA(hKey, "29");
            if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND) {
                success = true;
            } else {
                char buf[100];
                wsprintfA(buf, "RegDeleteValueA failed with error: %d", status);
                MessageBoxA(hWnd, buf, "Error", MB_ICONERROR);
            }
        }
        RegCloseKey(hKey);
    } else {
        char buf[100];
        wsprintfA(buf, "RegCreateKeyExA failed with error: %d", status);
        MessageBoxA(hWnd, buf, "Error", MB_ICONERROR);
    }

    if (success) {
        int choice = MessageBoxA(
            hWnd,
            "Registry updated successfully!\n\nWindows Explorer must be restarted to apply the changes. Would you like to restart Explorer now?",
            "Restart Required", MB_YESNO | MB_ICONQUESTION
        );
        if (choice == IDYES) {
            HWND hWndTray = FindWindowA("Shell_TrayWnd", NULL);
            if (hWndTray) PostMessageA(hWndTray, WM_USER + 436, 0, 0);
            Sleep(1500);
            WinExec("explorer.exe", SW_SHOW);
        } else {
            ShowWindow(hWnd, SW_HIDE);
            MessageBoxA(
                NULL,
                "Changes will apply on next boot or when Windows Explorer is restarted.",
                "Notice", MB_OK | MB_ICONINFORMATION
            );
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

            Rectangle(hdc, 15, 20, 135, 55);
            TextOutA(hdc, 32, 28, "Remove Arrows", 13);

            Rectangle(hdc, 155, 20, 275, 55);
            TextOutA(hdc, 172, 28, "Restore Default", 15);

            EndPaint(hWnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            if (x >= 15 && x <= 135 && y >= 20 && y <= 55) {
                ToggleArrows(true, hWnd);
                ExitProcess(0);
            } else if (x >= 155 && x <= 275 && y >= 20 && y <= 55) {
                ToggleArrows(false, hWnd);
                ExitProcess(0);
            }
            break;
        }
        case WM_DESTROY:
            ExitProcess(0);
        default:
            return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
    return 0;
}

extern "C" void __stdcall WinMainCRTStartup() {
    HINSTANCE hInst = GetModuleHandleA(NULL);

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "µUI";

    RegisterClassExA(&wc);

    HWND hWnd = CreateWindowExA(
        WS_EX_TOPMOST, wc.lpszClassName, "DeArrow",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT,
        CW_USEDEFAULT, 300, 115, NULL, NULL, hInst, NULL
    );

    ShowWindow(hWnd, SW_SHOW);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        DispatchMessageA(&msg);
    }
}