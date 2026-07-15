#include <shlobj.h>
#include <windows.h>

#ifdef CLI_MODE
char ToLower(char c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

bool StrContains(const char *haystack, const char *needle) {
    if (!haystack || !needle) return false;
    for (int i = 0; haystack[i] != '\0'; i++) {
        int j = 0;
        while (needle[j] != '\0' && ToLower(haystack[i + j]) == ToLower(needle[j])) j++;
        if (needle[j] == '\0') return true;
    }
    return false;
}

const char *GetArgs(const char *cmdLine) {
    if (!cmdLine) return "";
    bool inQuotes = false;
    const char *p = cmdLine;
    while (*p != '\0') {
        if (*p == '\"') inQuotes = !inQuotes;
        else if (*p == ' ' && !inQuotes) {
            while (*p == ' ') p++;
            return p;
        }
        p++;
    }
    return "";
}

void ConsolePrint(const char *text) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    bool mustClose = false;
    if (hOut == NULL || hOut == INVALID_HANDLE_VALUE) {
        hOut = CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        mustClose = true;
    }
    if (hOut != INVALID_HANDLE_VALUE && hOut != NULL) {
        DWORD written;
        WriteFile(hOut, text, lstrlenA(text), &written, NULL);
        if (mustClose) CloseHandle(hOut);
    }
}

HANDLE GetConsoleInputHandle(bool &mustClose) {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    mustClose = false;
    if (hIn == NULL || hIn == INVALID_HANDLE_VALUE) {
        hIn = CreateFileA("CONIN$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        mustClose = true;
    }
    return hIn;
}

void ConsoleInput(char *buf, DWORD maxLen) {
    bool mustClose = false;
    HANDLE hIn = GetConsoleInputHandle(mustClose);
    DWORD read = 0;
    if (hIn != INVALID_HANDLE_VALUE && hIn != NULL) {
        ReadConsoleA(hIn, buf, maxLen - 1, &read, NULL);
        buf[read] = '\0';
        while (read > 0 && (buf[read - 1] == '\r' || buf[read - 1] == '\n' || buf[read - 1] == ' ')) {
            buf[read - 1] = '\0';
            read--;
        }
    }
    if (mustClose && hIn != INVALID_HANDLE_VALUE && hIn != NULL) CloseHandle(hIn);
}

void WaitForKeypress() {
    bool mustClose = false;
    HANDLE hIn = GetConsoleInputHandle(mustClose);
    if (hIn != INVALID_HANDLE_VALUE && hIn != NULL) {
        INPUT_RECORD record;
        DWORD read;
        while (ReadConsoleInputA(hIn, &record, 1, &read)) {
            if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) break;
        }
    }
    if (mustClose && hIn != INVALID_HANDLE_VALUE && hIn != NULL) CloseHandle(hIn);
}
#endif

void ToggleArrows(bool remove, HWND hWnd = NULL) {
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
#ifndef CLI_MODE
                char buf[100];
                wsprintfA(buf, "RegSetValueExA failed with error: %d", status);
                MessageBoxA(hWnd, buf, "Error", MB_ICONERROR);
#else
                ConsolePrint("Error: Failed to write registry value.\n");
#endif
            }
        } else {
            status = RegDeleteValueA(hKey, "29");
            if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND) {
                success = true;
            } else {
#ifndef CLI_MODE
                char buf[100];
                wsprintfA(buf, "RegDeleteValueA failed with error: %d", status);
                MessageBoxA(hWnd, buf, "Error", MB_ICONERROR);
#else
                ConsolePrint("Error: Failed to delete registry value.\n");
#endif
            }
        }
        RegCloseKey(hKey);
    } else {
#ifndef CLI_MODE
        char buf[100];
        wsprintfA(buf, "RegCreateKeyExA failed with error: %d", status);
        MessageBoxA(hWnd, buf, "Error", MB_ICONERROR);
#else
        ConsolePrint("Error: Access denied. Make sure you are running as administrator.\n");
#endif
    }

    if (success) {
#ifdef CLI_MODE
        char choice[10];
        while (true) {
            ConsolePrint("\nRegistry updated successfully!\n");
            ConsolePrint("Would you like to restart Windows Explorer now to apply changes? (y/n): ");
            ConsoleInput(choice, sizeof(choice));
            if (StrContains(choice, "y")) {
                HWND hWndTray = FindWindowA("Shell_TrayWnd", NULL);
                if (hWndTray) PostMessageA(hWndTray, WM_USER + 436, 0, 0);
                Sleep(1500);
                WinExec("explorer.exe", SW_SHOW);
                break;
            } else if (StrContains(choice, "n")) {
                ConsolePrint("\nChanges will apply on next boot or when Windows Explorer is restarted.");
                ConsolePrint("\nPress any key to close...");
                WaitForKeypress();
                break;
            } else {
                ConsolePrint("Invalid choice. Please type 'y' or 'n'.\n");
            }
        }
#else
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
#endif
    }
}

#ifndef CLI_MODE
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
#endif

extern "C" void __stdcall WinMainCRTStartup() {
#ifdef CLI_MODE
    ConsolePrint("> DeArrow CLI\n");
    ConsolePrint(">> Made by falconwasplaying\n");
    ConsolePrint("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

    char action[50];
    while (true) {
        ConsolePrint("Type 'remove' to hide shortcut arrows, or 'restore' to show them: ");
        ConsoleInput(action, sizeof(action));

        if (StrContains(action, "remove")) {
            ToggleArrows(true);
            break;
        } else if (StrContains(action, "restore")) {
            ToggleArrows(false);
            break;
        } else {
            ConsolePrint("Invalid Input!\n\n");
        }
    }
    ExitProcess(0);
#else
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
#endif
}