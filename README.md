# DeArrow

A lightweight portable Windows utility designed to toggle shortcut overlay arrows on and off.

It is completely native, contains _zero_ external dependencies, runs with a very low memory footprint, using 1.2 MB of ram for the GUI tool and 0.6* MB for the CLI tool, your numbers may vary. (*In the case of the CLI tool, Task Manager may show a larger number because of Console Window Host taking extra memory.)

## Features
* **Two Formats Available**:
  * **`DeArrowGUI.exe`**: A minimalist, mouse-driven graphical interface (only 7.0 KB).
  * **`DeArrowCLI.exe`**: An interactive command-line tool (only 6.5 KB).

## System Requirements
* **Operating System**: Windows 7, Windows 8.1, Windows 10, or Windows 11.
* **Architecture**: 64-bit (x86-64) only (will not run on 32-bit (x86-32) versions of Windows).
* **Permissions**: Administrator privileges are _required_ for modifying system-wide icon configurations which requires writing to `HKEY_LOCAL_MACHINE`.

## How to Use

### Graphical User Interface (`DeArrowGUI.exe`)
1. Run `DeArrowGUI.exe`.
2. Accept the User Account Control (UAC) prompt to run as Administrator.
3. Click **Remove Arrows** to hide them, or **Restore Default** to bring them back.
4. Click **Yes** when prompted to restart Windows Explorer to apply changes immediately.

### Command-Line Interface (`DeArrowCLI.exe`)
1. Run `DeArrowCLI.exe` (will run directly in your console window if you run it through cmd with admin previleges).
2. Accept the UAC prompt to run as Administrator.
3. Type `remove` or `restore` (case-insensitive) and press **Enter**.
4. Type `y` to restart Windows Explorer now, or `n` to exit 

> If you choose to not restart Windows Explorer immediately, the changes may remain until next boot or whenever Windows Explorer is restarted, whether that's manually or not. This applies to both the CLI tool and the GUI application.

---

## How to Build

The project is built using GCC (MinGW-w64) in MSYS2.

1. Clone the repository to your Windows machine.
2. Open the folder where you've cloned the repository.
3. Run the build script for the version you want:
   * Run **`buildGUI.bat`** to compile the graphical application (`DeArrowGUI.exe`).
   * Run **`buildCLI.bat`** to compile the command-line tool (`DeArrowCLI.exe`).

The build scripts are made to compile with important compiler flags (`-Os`, `-s`, `-ffunction-sections`, `-fdata-sections`, and custom entry points) to ensure the executables remain resourceful.
