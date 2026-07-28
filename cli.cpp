#define _WIN32_WINNT 0x0600
#include <shlobj.h>
#include <windows.h>

bool ExtractBlankIcon(char *outPath, DWORD maxLen) {
  char appData[MAX_PATH];
  if (ExpandEnvironmentStringsA("%LOCALAPPDATA%\\DeArrow", appData,
                                sizeof(appData)) == 0) {
    return false;
  }

  CreateDirectoryA(appData, NULL);
  SetFileAttributesA(appData, FILE_ATTRIBUTE_HIDDEN);

  wsprintfA(outPath, "%s\\blank.ico", appData);

  HRSRC hRes = FindResourceA(NULL, MAKEINTRESOURCE(101), RT_RCDATA);
  if (hRes == NULL)
    return false;

  HGLOBAL hData = LoadResource(NULL, hRes);
  if (hData == NULL)
    return false;

  DWORD size = SizeofResource(NULL, hRes);
  void *pData = LockResource(hData);
  if (pData == NULL || size == 0)
    return false;

  SetFileAttributesA(outPath, FILE_ATTRIBUTE_NORMAL);
  HANDLE hFile = CreateFileA(outPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                             FILE_ATTRIBUTE_HIDDEN, NULL);
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
  if (ExpandEnvironmentStringsA("%LOCALAPPDATA%\\DeArrow\\blank.ico",
                                appDataPath, sizeof(appDataPath)) > 0) {
    SetFileAttributesA(appDataPath, FILE_ATTRIBUTE_NORMAL);
    DeleteFileA(appDataPath);
  }
  if (ExpandEnvironmentStringsA("%LOCALAPPDATA%\\DeArrow", appDataPath,
                                sizeof(appDataPath)) > 0) {
    RemoveDirectoryA(appDataPath);
  }
}

bool EnablePrivilege(const char *privilegeName) {
  HANDLE hToken = NULL;
  if (OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
    TOKEN_PRIVILEGES tp;
    LUID luid;
    if (LookupPrivilegeValueA(NULL, privilegeName, &luid)) {
      tp.PrivilegeCount = 1;
      tp.Privileges[0].Luid = luid;
      tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL,
                            NULL);
    }
    CloseHandle(hToken);
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
      hExplorerProc =
          OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwPID);
      if (hExplorerProc != NULL) {
        HANDLE hToken = NULL;
        if (OpenProcessToken(hExplorerProc,
                             TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY |
                                 TOKEN_QUERY,
                             &hToken)) {
          DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL,
                           SecurityImpersonation, TokenPrimary, &hNewToken);
          CloseHandle(hToken);
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
    STARTUPINFOW si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};
    wchar_t cmd[MAX_PATH];
    if (ExpandEnvironmentStringsW(L"%SystemRoot%\\explorer.exe", cmd,
                                  MAX_PATH) == 0) {
      lstrcpyW(cmd, L"C:\\Windows\\explorer.exe");
    }
    if (CreateProcessWithTokenW(hNewToken, LOGON_WITH_PROFILE, NULL, cmd, 0,
                                NULL, NULL, &si, &pi)) {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    } else {
      WinExec("explorer.exe", SW_SHOW);
    }
    CloseHandle(hNewToken);
  } else {
    WinExec("explorer.exe", SW_SHOW);
  }
}

void ClearScreen() {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;
  DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
  COORD topLeft = {0, 0};
  DWORD written;
  FillConsoleOutputCharacterA(hOut, ' ', cellCount, topLeft, &written);
  FillConsoleOutputAttribute(hOut, csbi.wAttributes, cellCount, topLeft, &written);
  SetConsoleCursorPosition(hOut, topLeft);
}

void PrintStr(const char *str) {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut && hOut != INVALID_HANDLE_VALUE) {
    DWORD written = 0;
    WriteFile(hOut, str, lstrlenA(str), &written, NULL);
  }
}

bool ReadLine(char *buf, DWORD maxLen) {
  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
  if (!hIn || hIn == INVALID_HANDLE_VALUE)
    return false;

  DWORD read = 0;
  if (!ReadFile(hIn, buf, maxLen - 1, &read, NULL) || read == 0) {
    return false;
  }
  buf[read] = '\0';
  for (DWORD i = 0; i < read; i++) {
    if (buf[i] == '\r' || buf[i] == '\n') {
      buf[i] = '\0';
      break;
    }
  }
  return true;
}

bool StringEqualsIgnoreCase(const char *a, const char *b) {
  while (*a && *b) {
    char ca = (*a >= 'A' && *a <= 'Z') ? (*a + 32) : *a;
    char cb = (*b >= 'A' && *b <= 'Z') ? (*b + 32) : *b;
    if (ca != cb)
      return false;
    a++;
    b++;
  }
  return (*a == '\0' && *b == '\0');
}

void WaitKey() {
  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
  if (hIn && hIn != INVALID_HANDLE_VALUE) {
    DWORD mode = 0;
    GetConsoleMode(hIn, &mode);
    SetConsoleMode(hIn, 0);
    INPUT_RECORD rec;
    DWORD read = 0;
    while (ReadConsoleInputA(hIn, &rec, 1, &read)) {
      if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
        break;
      }
    }
    SetConsoleMode(hIn, mode);
  }
}

bool ToggleArrows(bool remove) {
  HKEY hKey;
  LSTATUS status = RegCreateKeyExA(
      HKEY_LOCAL_MACHINE,
      "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Icons", 0,
      NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey,
      NULL);

  bool success = false;
  if (status == ERROR_SUCCESS) {
    if (remove) {
      char iconPath[MAX_PATH];
      char regValue[MAX_PATH + 10];
      if (ExtractBlankIcon(iconPath, sizeof(iconPath))) {
        wsprintfA(regValue, "%s,0", iconPath);
        status = RegSetValueExA(hKey, "29", 0, REG_SZ, (const BYTE *)regValue,
                                lstrlenA(regValue) + 1);
        if (status == ERROR_SUCCESS) {
          success = true;
        } else {
          PrintStr("\nERROR: Failed to write registry value.\n");
        }
      } else {
        PrintStr("\nERROR: Failed to create hidden blank icon file.\n");
      }
    } else {
      status = RegDeleteValueA(hKey, "29");
      if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND) {
        success = true;
      } else {
        PrintStr("\nERROR: Failed to delete registry value.\n");
      }
    }
    RegCloseKey(hKey);
  } else {
    PrintStr("\nERROR: Failed to open registry key.\n");
  }

  if (success) {
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
  }
  return success;
}

extern "C" void __stdcall mainCRTStartup() {
  char input[64];

  while (true) {
    PrintStr("> Type 'remove' (or 'rm') to hide shortcut arrows, 'restore' (or 'rs') to show them, or 'exit' to exit.\n\n");
    PrintStr("> Choice: ");

    if (!ReadLine(input, sizeof(input))) {
      break;
    }

    if (StringEqualsIgnoreCase(input, "exit")) {
      ExitProcess(0);
    }

    bool remove = false;
    if (StringEqualsIgnoreCase(input, "remove") || StringEqualsIgnoreCase(input, "rm")) {
      remove = true;
    } else if (StringEqualsIgnoreCase(input, "restore") || StringEqualsIgnoreCase(input, "rs")) {
      remove = false;
    } else {
      PrintStr("\nInvalid Input!\n");
      Sleep(1500);
      ClearScreen();
      continue;
    }

    if (ToggleArrows(remove)) {
      while (true) {
        PrintStr("\nRegistry updated successfully!\n\n");
        PrintStr("> Would you like to restart Windows Explorer now to apply "
                 "changes? (y/n): ");

        if (!ReadLine(input, sizeof(input))) {
          ExitProcess(0);
        }

        if (StringEqualsIgnoreCase(input, "y")) {
          RestartExplorerUnelevated();
          if (!remove) {
            RemoveBlankIconFile();
          }
          ExitProcess(0);
        } else if (StringEqualsIgnoreCase(input, "n")) {
          PrintStr("\n> Changes will apply on next boot or when windows "
                   "explorer is restarted\n\n");
          PrintStr("> Press any key to close...\n");
          WaitKey();
          if (!remove) {
            RemoveBlankIconFile();
          }
          ExitProcess(0);
        } else {
          PrintStr("\nInvalid choice. Please type 'y' or 'n'.\n");
        }
      }
    } else {
      PrintStr("\nPress any key to close...\n");
      WaitKey();
      ExitProcess(1);
    }
  }
  ExitProcess(0);
}
