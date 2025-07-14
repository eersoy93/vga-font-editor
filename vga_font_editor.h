#ifndef VGA_FONT_EDITOR_H
#define VGA_FONT_EDITOR_H

#include <windows.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <dwmapi.h>

// Link UXTHEME and DWM libraries (GCC uses -l flags in Makefile instead of pragma)

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
#define ID_EDIT_COPY 1005
#define ID_EDIT_PASTE 1006
#define ID_EDIT_CLEAR 1007
#define ID_EDIT_UNDO 1008
#define ID_EDIT_REDO 1009

// Status bar and toolbar IDs
#define ID_STATUSBAR 3000
#define ID_TOOLBAR 3001
#define ID_TB_CLEAR 3002
#define ID_TB_COPY 3003
#define ID_TB_PASTE 3004
#define ID_TB_ZOOM_IN 3005
#define ID_TB_ZOOM_OUT 3006

// UI Constants
#define STATUSBAR_HEIGHT 25
#define MIN_ZOOM 1
#define MAX_ZOOM 2

// Modern UI Colors - Güncellenmiş modern renk paleti
#define COLOR_MODERN_BG RGB(248, 249, 250)
#define COLOR_MODERN_BORDER RGB(218, 220, 224)
#define COLOR_MODERN_HIGHLIGHT RGB(66, 133, 244)
#define COLOR_MODERN_HOVER RGB(241, 243, 244)
#define COLOR_MODERN_TEXT RGB(32, 33, 36)
#define COLOR_MODERN_GRID RGB(189, 193, 198)
#define COLOR_MODERN_SELECTED RGB(232, 240, 254)
#define COLOR_MODERN_PIXEL RGB(60, 64, 67)

// Ek modern renkler
#define COLOR_MODERN_TOOLBAR RGB(255, 255, 255)
#define COLOR_MODERN_TOOLBAR_BORDER RGB(218, 220, 224)
#define COLOR_MODERN_BUTTON RGB(248, 249, 250)
#define COLOR_MODERN_BUTTON_HOVER RGB(232, 240, 254)
#define COLOR_MODERN_BUTTON_PRESSED RGB(66, 133, 244)
#define COLOR_MODERN_ACCENT RGB(52, 168, 83)
#define COLOR_MODERN_WARNING RGB(251, 188, 5)
#define COLOR_MODERN_ERROR RGB(234, 67, 53)
#define COLOR_MODERN_SHADOW RGB(0, 0, 0)
#define COLOR_MODERN_WHITE RGB(255, 255, 255)
#define COLOR_MODERN_SECONDARY_TEXT RGB(95, 99, 104)

// Toolbar yüksekliği modern tasarım için artırıldı
#define TOOLBAR_HEIGHT 48

// VGA Font structure
typedef struct {
    unsigned char data[VGA_FONT_CHARS][VGA_CHAR_HEIGHT];
} VGAFont;

// Clipboard structure for character copying
typedef struct {
    unsigned char data[VGA_CHAR_HEIGHT];
    BOOL hasData;
} CharClipboard;

// Global variables (declared in main.c)
extern HINSTANCE g_hInstance;
extern HWND g_hMainWindow;
extern VGAFont g_font;
extern int g_selectedChar;
extern HWND g_hCharGrid, g_hPixelEditor, g_hCharInfo;
extern HWND g_hStatusBar, g_hToolbar;
extern CharClipboard g_clipboard;
extern int g_zoomLevel;

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

// Clipboard operations
void CopyCharacterToClipboard(int charIndex);
void PasteCharacterFromClipboard(int charIndex);

// UI functions
void UpdateCharacterInfo();
void InvalidateCharacter(int charIndex);
void UpdateStatusBar();
void CreateStatusBar(HWND hwnd);

// Window procedures for custom controls
LRESULT CALLBACK CharGridProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PixelEditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Drawing functions
void DrawVGACharacter(HDC hdc, int x, int y, int width, int height, VGAFont* font, int charIndex);

// UXTHEME support functions
void InitializeTheming();
void EnableVisualStyles();
BOOL IsThemingEnabled();
void ApplyModernStyling(HWND hwnd);
void DrawThemedButton(HDC hdc, RECT* rect, int state, const char* text);
void DrawThemedBackground(HDC hdc, RECT* rect, int partId, int stateId);

#endif // VGA_FONT_EDITOR_H
