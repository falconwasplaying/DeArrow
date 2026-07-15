# DeArrow

A lightweight, portable Windows utility designed to toggle shortcut overlay arrows on and off. 

It is completely native, contains **zero external dependencies**, runs with a near-zero memory footprint (<1MB), and does not require installation.

## Features
* **Two Formats Available**:
  * **`DeArrowGUI.exe`**: A minimalist, mouse-driven graphical interface (only 7.0 KB).
  * **`DeArrowCLI.exe`**: An interactive command-line wizard (only 6.5 KB).
* **Graceful Explorer Restarts**: Offers to restart Windows Explorer programmatically via standard shell broadcasts, avoiding sluggish manual shell resets.
* **HKLM Registry Target**: Modifies registry settings at the machine level (`HKEY_LOCAL_MACHINE`), ensuring compatibility with Windows 10 and Windows 11 which ignore user-level shortcut customisations.

---

## System Requirements
* **Operating System**: Windows 7, Windows 8.1, Windows 10, or Windows 11.
* **Architecture**: **64-bit (x64) only**. (Will not run on 32-bit/x86 versions of Windows).
* **Permissions**: **Administrator privileges** are required because modifying system-wide icon configurations requires writing to `HKEY_LOCAL_MACHINE`. The executables will automatically request admin credentials (UAC prompt) on launch.

---

## How to Use

### Graphical User Interface (`DeArrowGUI.exe`)
1. Double-click `DeArrowGUI.exe`.
2. Accept the User Account Control (UAC) prompt to run as Administrator.
3. Click **Remove Arrows** to hide them, or **Restore Default** to bring them back.
4. Click **Yes** when prompted to restart Windows Explorer to apply changes immediately.

### Command-Line Interface (`DeArrowCLI.exe`)
1. Double-click `DeArrowCLI.exe` to run the interactive wizard, or run it from an elevated (Administrator) Command Prompt.
2. Accept the UAC prompt.
3. Type `remove` or `restore` (case-insensitive) and press **Enter**.
4. Type `y` to restart Windows Explorer now, or `n` to exit.

---

## How to Build

The project is built using GCC (MinGW-w64) in MSYS2.

1. Open the project folder.
2. Run the build script for the version you want:
   * Run **`buildGUI.bat`** to compile the graphical application (`DeArrowGUI.exe`).
   * Run **`buildCLI.bat`** to compile the command-line wizard (`DeArrowCLI.exe`).

The build scripts compile with aggressive size-optimization compiler flags (`-Os`, `-s`, `-ffunction-sections`, `-fdata-sections`, and custom entry points) to ensure the executables remain extremely small.
