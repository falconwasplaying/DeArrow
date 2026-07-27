# DeArrow

A simple lightweight portable Windows utility designed to toggle shortcut overlay arrows on and off, made to be as optimised as can be and is completely native to Windows, containing _zero_ external dependencies, thus only taking 7.5 KB of storage space for the 64-bit exe and 1.59 KB for the CLI tool, also able to run with a very low memory footprint, using 1.2 MB of ram for the GUI tool, your numbers may vary.

## System Requirements

* **Operating System**: Windows 7, Windows 8.1, Windows 10, or Windows 11.
* **Architecture**: 64-bit (x86-64) and 32-bit (x86-32).
* **Permissions**: Administrator privileges are _required_ for modifying system-wide icon configurations which requires writing to `HKEY_LOCAL_MACHINE`.

## How to Use:

* **Two Formats Available**:
  * **`dearrow-x64.exe/dearrow-x32.exe`**: A simple graphical interface.
  * **`dearrow-cli-x64.exe/dearrow-cli-x32.exe`**: An interactive command-line tool.

### Graphical User Interface (`dearrow-x64.exe/dearrow-x32.exe`)
1. Run `dearrow-x64.exe/dearrow-x32.exe`.
2. Accept the User Account Control (UAC) prompt to run as Administrator.
3. Click **Remove Arrows** to hide them, or **Restore Default** to bring them back.
4. Click **Yes** when prompted to restart Windows Explorer to apply changes immediately, or **No** to exit.

### Command-Line Interface (`dearrow-cli.bat`)
1. Run `dearrow-cli.bat` (will run directly in your console window if you run it through cmd with admin previleges).
2. Accept the UAC prompt to run as Administrator.
3. Type `remove` or `restore` (case-insensitive) and press **Enter**.
4. Type `y` to restart Windows Explorer now, or `n` to exit.

> If you choose to not restart Windows Explorer immediately, the changes may remain until next boot or whenever Windows Explorer is restarted, whether that's manually or not. This applies to both the CLI tool and the GUI application.

---

## How to Build the GUI App Yourself

You'll need MinGW-w64/UCRT64 and MinGW-w32 installed (via [MSYS2](https://www.msys2.org/) is easiest).

### 64-bit build

Open the **MSYS2 UCRT64** (or **MINGW64**) terminal, and go to your project directory:

```
cd /c/users/username/documents/dearrow
```
> replace "/c/users/username/documents/dearrow" with your project directory

Now run:

```
windres resources-x64.rc -O coff -o resources-x64.o
```
```
g++ main.cpp resources-x64.o -o dearrow-x64.exe -mwindows -lgdi32 -ladvapi32 -lshell32 -nostartfiles -e WinMainCRTStartup -Os -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections "-Wl,--gc-sections"
```

### 32-bit build

Open the **MINGW32** terminal, and go to your project directory:

```
cd /c/users/username/documents/dearrow
```
> replace "/c/users/username/documents/dearrow" with your project directory

Now run:
```
windres resources-x32.rc -O coff -o resources-x32.o
```
```
g++ main.cpp resources-x32.o -o dearrow-x32.exe -mwindows -lgdi32 -ladvapi32 -lshell32 -nostartfiles -e _WinMainCRTStartup@0 -Os -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -static-libgcc -Wl,--gc-sections
```

> Each MSYS2 shell (UCRT64/MINGW64/MINGW32) automatically points `g++` and
> `windres` at the matching architecture — you don't need to type prefixed
> binary names or full paths, just make sure you're in the right shell.

### If not using MSYS2

If you installed MinGW-w64 another way (e.g. WinLibs) and have both
architectures available, use the prefixed binaries explicitly instead:

```bash
# 64-bit
x86_64-w64-mingw32-windres resources.rc -O coff -o resources.o
x86_64-w64-mingw32-g++ main.cpp resources.o -o dearrow-x64.exe -mwindows -lgdi32 -ladvapi32 -lshell32 -nostartfiles -e WinMainCRTStartup -Os -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections "-Wl,--gc-sections"

# 32-bit
i686-w64-mingw32-windres resources.rc -O coff -o resources.o
i686-w64-mingw32-g++ main.cpp resources.o -o dearrow-x32.exe -mwindows -lgdi32 -ladvapi32 -lshell32 -nostartfiles -e WinMainCRTStartup -Os -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections "-Wl,--gc-sections"
```
