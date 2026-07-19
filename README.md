# DeArrow

A simple lightweight portable Windows utility designed to toggle shortcut overlay arrows on and off, made to be as optimised as can be and is completely native to Windows, containing _zero_ external dependencies, thus only taking 7.5 KB of storage space for the GUI tool and 1.59 KB for the CLI tool, also able to run with a very low memory footprint, using 1.2 MB of ram for the GUI tool, your numbers may vary.

## System Requirements

* **Operating System**: Windows 7, Windows 8.1, Windows 10, or Windows 11.
* **Architecture**: 64-bit (x86-64) for the GUI version (as of now), the CLI version will run on both 32-bit (x86-32) and 64-bit (x86-64) Windows versions.
* **Permissions**: Administrator privileges are _required_ for modifying system-wide icon configurations which requires writing to `HKEY_LOCAL_MACHINE`.

## How to Use:

* **Two Formats Available**:
  * **`DeArrow.exe`**: A simple graphical interface (only 7.5 KB).
  * **`DeArrowCLI.bat`**: An interactive command-line tool (only 1.59 KB).

### Graphical User Interface (`DeArrowGUI.exe`)
1. Run `DeArrowGUI.exe`.
2. Accept the User Account Control (UAC) prompt to run as Administrator.
3. Click **Remove Arrows** to hide them, or **Restore Default** to bring them back.
4. Click **Yes** when prompted to restart Windows Explorer to apply changes immediately, or **No** to exit.

### Command-Line Interface (`DeArrowCLI.bat`)
1. Run `DeArrowCLI.bat` (will run directly in your console window if you run it through cmd with admin previleges).
2. Accept the UAC prompt to run as Administrator.
3. Type `remove` or `restore` (case-insensitive) and press **Enter**.
4. Type `y` to restart Windows Explorer now, or `n` to exit.

> If you choose to not restart Windows Explorer immediately, the changes may remain until next boot or whenever Windows Explorer is restarted, whether that's manually or not. This applies to both the CLI tool and the GUI application.

---

## How to Build the GUI App Yourself

The project is built using GCC (MinGW-w64) in MSYS2.

1. Clone the repository to your Windows machine or download the source code zip from the releases section.
2. Open the folder where you've cloned the repository.
3. Run **`build.bat`** to compile the app (`DeArrow.exe`).
4. When done, press any key to exit.

The build script is made to compile with important compiler flags (`-Os`, `-s`, `-ffunction-sections`, `-fdata-sections`, and custom entry points) to ensure the executable remains resourceful.
