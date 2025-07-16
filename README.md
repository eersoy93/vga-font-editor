# VGA Font Editor

A simple VGA text mode font editor for Windows, written in C using Windows API.

## Features

- Edit 8x16 pixel VGA fonts (256 characters)
- Visual character grid showing all 256 characters
- Pixel-level character editor
- Load and save VGA font files (.vgaf format)
- Mouse and keyboard support for pixel editing
- Character selection and navigation

## Building

This project is designed to be compiled with MinGW-w64. Make sure you have MinGW-w64 installed and added to your PATH.

### Prerequisites

- MinGW-w64 (with gcc and windres)
- Make (optional, for using Makefile)

### Compilation

#### Using Makefile:
```bash
make
```

#### Manual compilation:
```bash
# Compile resource file
windres -i resource.rc -o resource.o

# Compile C files
gcc -Wall -Wextra -std=c99 -O2 -c main.c -o main.o
gcc -Wall -Wextra -std=c99 -O2 -c font_manager.c -o font_manager.o
gcc -Wall -Wextra -std=c99 -O2 -c controls.c -o controls.o

# Link executable
gcc -mwindows -o vga-font-editor.exe main.o font_manager.o controls.o resource.o -lgdi32 -luser32 -lkernel32 -lcomdlg32
```

## Usage

1. **Character Selection**: Click on any character in the grid (left panel) to select it for editing.

2. **Pixel Editing**: 
   - Left click in the pixel editor (right panel) to set pixels (black)
   - Right click to clear pixels (white)
   - Drag with mouse button held to draw/erase multiple pixels

3. **Keyboard Shortcuts**:
   - `Delete` or `Backspace`: Clear the current character
   - `Ctrl+O`: Open font file
   - `Ctrl+S`: Save font file

4. **File Operations**:
   - Use File → Open to load existing VGA font files
   - Use File → Save to save your edited font
   - Font files are saved in VGAF format (.vgaf)

## File Format

The editor uses the VGAF (VGA Font) format:
- 16-byte header with format identification and metadata
- 256 characters × 16 bytes per character = 4096 bytes font data
- Total file size: 4112 bytes
- Each character is 8 pixels wide × 16 pixels tall
- Each byte represents one row of pixels (MSB = leftmost pixel)
- Header contains version info, character dimensions, and format validation

## Character Set

The editor supports the full PC/VGA character set (characters 0-255):
- 0-31: Control characters and symbols
- 32-126: Standard ASCII printable characters
- 127-255: Extended ASCII and graphics characters

## Contributing

Feel free to submit issues and enhancement requests!

## License

Copyright (c) 2025 Erdem Ersoy (eersoy93)

This project is licensed under the GPLv3 License - see the LICENSE file for details.
