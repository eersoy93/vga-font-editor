#ifndef VGA_FONT_EDITOR_H
#define VGA_FONT_EDITOR_H

#include <windows.h>

// Constants
#define VGA_CHAR_WIDTH 8
#define VGA_CHAR_HEIGHT 16
#define VGA_FONT_CHARS 256

// Menu IDs
#define IDR_MENU 1000
#define ID_FILE_OPEN 1001
#define ID_FILE_SAVE 1002
#define ID_FILE_EXIT 1003
#define ID_HELP_ABOUT 1004

// VGA Font structure
typedef struct {
    unsigned char data[VGA_FONT_CHARS][VGA_CHAR_HEIGHT];
} VGAFont;

// Global variables (declared in main.c)
extern HINSTANCE g_hInstance;
extern HWND g_hMainWindow;
extern VGAFont g_font;
extern int g_selectedChar;
extern HWND g_hCharGrid, g_hPixelEditor, g_hCharInfo;

// Function prototypes

// Font management
void InitializeVGAFont(VGAFont* font);
void LoadBasicASCIIChars(VGAFont* font);
BOOL LoadVGAFont(VGAFont* font, const char* filename);
BOOL SaveVGAFont(VGAFont* font, const char* filename);

// Character manipulation
void SetFontPixel(VGAFont* font, int charIndex, int x, int y, BOOL value);
BOOL GetFontPixel(VGAFont* font, int charIndex, int x, int y);
void ClearCharacter(VGAFont* font, int charIndex);
void CopyCharacter(VGAFont* font, int srcChar, int dstChar);

// UI functions
void UpdateCharacterInfo();
void InvalidateCharacter(int charIndex);

// Window procedures for custom controls
LRESULT CALLBACK CharGridProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PixelEditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Drawing functions
void DrawVGACharacter(HDC hdc, int x, int y, int width, int height, VGAFont* font, int charIndex);

#endif // VGA_FONT_EDITOR_H
