#define _WIN32_WINNT 0x0600
#include <shlobj.h>
#include <windows.h>

bool ExtractBlankIcon(char *outPath, DWORD maxLen) {
    char appData[MAX_PATH];
    if (ExpandEnvironmentStringsA("%LOCALAPPDATA%\\DeArrow", appData, sizeof(appData)) == 0) {
        return false;
    }

    CreateDirectoryA(appData, NULL);
    SetFileAttributesA(appData, FILE_ATTRIBUTE_HIDDEN);

    wsprintfA(outPath, "%s\\blank.ico", appData);

    HRSRC hRes = FindResourceA(NULL, MAKEINTRESOURCE(101), RT_RCDATA);
    if (hRes == NULL) return false;

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (hData == NULL) return false;

    DWORD size = SizeofResource(NULL, hRes);
    void *pData = LockResource(hData);
    if (pData == NULL || size == 0) return false;

    SetFileAttributesA(outPath, FILE_ATTRIBUTE_NORMAL);
    HANDLE hFile = CreateFileA(outPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD written = 0;
    WriteFile(hFile, pData, size, &written, NULL);
    CloseHandle(hFile);

    return (written == size);
}

void RemoveBlankIconFile() {
    char appDataPath[MAX_PATH];
    if (ExpandEnvironmentStringsA("%LOCALAPPDATA%\\DeArrow\\blank.ico", appDataPath, sizeof(appDataPath)) > 0) {
        SetFileAttributesA(appDataPath, FILE_ATTRIBUTE_NORMAL);
        DeleteFileA(appDataPath);
    }
    if (ExpandEnvironmentStringsA("%LOCALAPPDATA%\\DeArrow", appDataPath, sizeof(appDataPath)) > 0) {
        RemoveDirectoryA(appDataPath);
    }
}

bool EnablePrivilege(const char *privilegeName) {
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        TOKEN_PRIVILEGES tp;
        LUID luid;
        if (LookupPrivilegeValueA(NULL, privilegeName, &luid)) {
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            SetLastError(0);
            AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
            DWORD err = GetLastError();
            if (err != ERROR_SUCCESS) {
                char buf[128];
                wsprintfA(buf, "AdjustTokenPrivileges(%s) returned error: %d", privilegeName, err);
                MessageBoxA(NULL, buf, "Diag: EnablePrivilege", MB_OK | MB_ICONWARNING);
            }
        }
        CloseHandle(hToken);
    } else {
        char buf[128];
        wsprintfA(buf, "OpenProcessToken(Self) failed: %d", GetLastError());
        MessageBoxA(NULL, buf, "Diag: OpenProcessToken Self", MB_OK | MB_ICONERROR);
    }
    return true;
}

void RestartExplorerUnelevated() {
    EnablePrivilege("SeImpersonatePrivilege");

    HWND hWndTray = FindWindowA("Shell_TrayWnd", NULL);
    HANDLE hNewToken = NULL;
    HANDLE hExplorerProc = NULL;

    if (hWndTray != NULL) {
        DWORD dwPID = 0;
        GetWindowThreadProcessId(hWndTray, &dwPID);
        if (dwPID != 0) {
            hExplorerProc = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwPID);
            if (hExplorerProc != NULL) {
                HANDLE hToken = NULL;
                if (OpenProcessToken(hExplorerProc, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &hToken)) {
                    if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hNewToken)) {
                        char buf[128];
                        wsprintfA(buf, "DuplicateTokenEx failed: %d", GetLastError());
                        MessageBoxA(NULL, buf, "Diag: DuplicateTokenEx", MB_OK | MB_ICONERROR);
                    }
                    CloseHandle(hToken);
                } else {
                    char buf[128];
                    wsprintfA(buf, "OpenProcessToken(Explorer) failed: %d", GetLastError());
                    MessageBoxA(NULL, buf, "Diag: OpenProcessToken Explorer", MB_ICONERROR);
                }
            }
        }
    }

    if (hWndTray != NULL) {
        PostMessageA(hWndTray, WM_USER + 436, 0, 0);
    }

    if (hExplorerProc != NULL) {
        WaitForSingleObject(hExplorerProc, 5000);
        CloseHandle(hExplorerProc);
    } else {
        Sleep(2000);
    }

    if (hNewToken != NULL) {
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi = { 0 };
        wchar_t cmd[MAX_PATH];
        if (ExpandEnvironmentStringsW(L"%SystemRoot%\\explorer.exe", cmd, MAX_PATH) == 0) {
            lstrcpyW(cmd, L"C:\\Windows\\explorer.exe");
        }
        SetLastError(0);
        if (CreateProcessWithTokenW(hNewToken, LOGON_WITH_PROFILE, NULL, cmd, 0, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            DWORD err = GetLastError();
            char buf[128];
            wsprintfA(buf, "CreateProcessWithTokenW failed: %d\nFalling back to WinExec...", err);
            MessageBoxA(NULL, buf, "Diag: CreateProcessWithTokenW", MB_OK | MB_ICONERROR);
            WinExec("explorer.exe", SW_SHOW);
        }
        CloseHandle(hNewToken);
    } else {
        MessageBoxA(NULL, "hNewToken was NULL! Falling back to WinExec...", "Diag: Token Null", MB_OK | MB_ICONWARNING);
        WinExec("explorer.exe", SW_SHOW);
    }
}

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
            char iconPath[MAX_PATH];
            char regValue[MAX_PATH + 10];
            if (ExtractBlankIcon(iconPath, sizeof(iconPath))) {
                wsprintfA(regValue, "%s,0", iconPath);
                status = RegSetValueExA(hKey, "29", 0, REG_SZ, (const BYTE *)regValue, lstrlenA(regValue) + 1);
                if (status == ERROR_SUCCESS) {
                    success = true;
                } else {
                    char buf[100];
                    wsprintfA(buf, "RegSetValueExA failed with error: %d", status);
                    MessageBoxA(hWnd, buf, "Error", MB_ICONERROR);
                }
            } else {
                MessageBoxA(hWnd, "Failed to create hidden blank icon file.", "Error", MB_ICONERROR);
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
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

        int choice = MessageBoxA(
            hWnd,
            "Registry updated successfully!\n\nWindows Explorer must be restarted to apply the changes. Would you like to restart Explorer now?",
            "Restart Required", MB_YESNO | MB_ICONQUESTION
        );
        if (choice == IDYES) {
            RestartExplorerUnelevated();
        } else {
            ShowWindow(hWnd, SW_HIDE);
            MessageBoxA(
                NULL,
                "Changes will apply on next boot or when Windows Explorer is restarted.",
                "Notice", MB_OK | MB_ICONINFORMATION
            );
        }

        if (!remove) {
            RemoveBlankIconFile();
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