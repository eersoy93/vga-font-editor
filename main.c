#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commctrl.h>
#include "vga_font_editor.h"

// Global variables
HINSTANCE g_hInstance;
HWND g_hMainWindow;
VGAFont g_font;
int g_selectedChar = 0;
HWND g_hCharGrid, g_hPixelEditor, g_hCharInfo;
HWND g_hStatusBar, g_hToolbar;
CharClipboard g_clipboard = {0};
int g_zoomLevel = 2; // Start with 2x zoom

// UXTHEME global variables
HTHEME g_hTheme = NULL;
BOOL g_bThemingEnabled = FALSE;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CharGridProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PixelEditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd);
void CreateToolbar(HWND hwnd);
void CreateStatusBar(HWND hwnd);
void LoadDefaultFont();
void UpdateCharacterInfo();
void UpdateStatusBar();
void CopyCharacterToClipboard(int charIndex);
void PasteCharacterFromClipboard(int charIndex);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Suppress unused parameter warnings
    (void)hPrevInstance;
    (void)lpCmdLine;
    
    g_hInstance = hInstance;
    
    // Initialize UXTHEME and enable visual styles
    InitializeTheming();
    EnableVisualStyles();
    
    // Initialize common controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);
    
    // Register window classes with modern styling
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "VGAFontEditor";
    wc.hbrBackground = CreateSolidBrush(COLOR_MODERN_BG); // Modern arka plan rengi
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
    wc.style = CS_HREDRAW | CS_VREDRAW; // Modern redraw style
    RegisterClass(&wc);
    
    // Register custom control classes with modern styling
    wc.lpfnWndProc = CharGridProc;
    wc.lpszClassName = "CharGrid";
    wc.hbrBackground = CreateSolidBrush(COLOR_MODERN_BG);
    wc.lpszMenuName = NULL;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    RegisterClass(&wc);
    
    wc.lpfnWndProc = PixelEditorProc;
    wc.lpszClassName = "PixelEditor";
    wc.hbrBackground = CreateSolidBrush(COLOR_MODERN_WHITE);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    RegisterClass(&wc);
    
    // Create main window with modern title and better size
    g_hMainWindow = CreateWindow(
        "VGAFontEditor",
        "VGA Font Editor - Copyright (c) 2025 Erdem Ersoy (eersoy93)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 800, // Increased height for better workspace
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hMainWindow) {
        MessageBox(NULL, "Failed to create main window", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    LoadDefaultFont();
    ShowWindow(g_hMainWindow, nCmdShow);
    UpdateWindow(g_hMainWindow);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CREATE:
            CreateControls(hwnd);
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_FILE_OPEN:
                    {
                        OPENFILENAME ofn;
                        char filename[MAX_PATH] = "";
                        
                        ZeroMemory(&ofn, sizeof(ofn));
                        ofn.lStructSize = sizeof(ofn);
                        ofn.hwndOwner = hwnd;
                        ofn.lpstrFile = filename;
                        ofn.nMaxFile = sizeof(filename);
                        ofn.lpstrFilter = 
                            "VGA Font Files (*.fnt)\0*.fnt\0"
                            "VGAF Files (*.vgaf)\0*.vgaf\0"
                            "PSF Font Files (*.psf)\0*.psf\0"
                            "Raw Binary (*.bin)\0*.bin\0"
                            "All Files (*.*)\0*.*\0";
                        ofn.nFilterIndex = 1;
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                        ofn.lpstrTitle = "Open Font File";
                        
                        if (GetOpenFileName(&ofn)) {
                            // First validate the file
                            if (!ValidateFontFile(filename)) {
                                char msg[512];
                                sprintf(msg, "Invalid font file format:\n\n"
                                           "File: %s\n"
                                           "Size: %ld bytes\n"
                                           "Format: %s\n\n"
                                           "Expected: VGA font data (4096 bytes minimum)",
                                           filename, GetFontFileSize(filename), 
                                           GetFontFormatName(filename));
                                MessageBox(hwnd, msg, "Invalid Font File", MB_OK | MB_ICONWARNING);
                                break;
                            }
                            
                            if (LoadVGAFont(&g_font, filename)) {
                                char msg[512];
                                sprintf(msg, "Font loaded successfully!\n\n"
                                           "File: %s\n"
                                           "Size: %ld bytes\n"
                                           "Format: %s",
                                           filename, GetFontFileSize(filename), 
                                           GetFontFormatName(filename));
                                
                                InvalidateRect(g_hCharGrid, NULL, TRUE);
                                InvalidateRect(g_hPixelEditor, NULL, TRUE);
                                UpdateStatusBar();
                                
                                // Show format info in status bar
                                if (g_hStatusBar) {
                                    SendMessage(g_hStatusBar, SB_SETTEXT, 3, (LPARAM)GetFontFormatName(filename));
                                }
                                
                                MessageBox(hwnd, msg, "Font Loaded", MB_OK | MB_ICONINFORMATION);
                            } else {
                                char msg[512];
                                sprintf(msg, "Failed to load font file:\n\n"
                                           "File: %s\n"
                                           "Format: %s\n\n"
                                           "The file may be corrupted or in an unsupported format.",
                                           filename, GetFontFormatName(filename));
                                MessageBox(hwnd, msg, "Load Error", MB_OK | MB_ICONERROR);
                            }
                        }
                    }
                    break;
                    
                case ID_FILE_SAVE:
                    {
                        OPENFILENAME ofn;
                        char filename[MAX_PATH] = "";
                        
                        ZeroMemory(&ofn, sizeof(ofn));
                        ofn.lStructSize = sizeof(ofn);
                        ofn.hwndOwner = hwnd;
                        ofn.lpstrFile = filename;
                        ofn.nMaxFile = sizeof(filename);
                        ofn.lpstrFilter = 
                            "VGAF Files (*.vgaf)\0*.vgaf\0"
                            "VGA Font Files (*.fnt)\0*.fnt\0"
                            "Raw Binary (*.bin)\0*.bin\0"
                            "C Header (*.h)\0*.h\0";
                        ofn.nFilterIndex = 1;
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
                        ofn.lpstrTitle = "Save Font File";
                        ofn.lpstrDefExt = "vgaf";
                        
                        if (GetSaveFileName(&ofn)) {
                            BOOL success = FALSE;
                            
                            // Check if saving as C header
                            char* ext = strrchr(filename, '.');
                            if (ext && strcmp(ext, ".h") == 0) {
                                // Export as C header
                                success = ExportToC(&g_font, filename, "vga_font_data");
                                if (success) {
                                    MessageBox(hwnd, 
                                        "Font exported as C header file successfully!\n\n"
                                        "You can now include this file in your C/C++ projects.",
                                        "Export Complete", MB_OK | MB_ICONINFORMATION);
                                } else {
                                    MessageBox(hwnd, "Failed to export C header file", "Export Error", MB_OK | MB_ICONERROR);
                                }
                            } else {
                                // Determine format based on extension and filter selection
                                char* ext = strrchr(filename, '.');
                                const char* formatName = "Unknown";
                                
                                if (ext && (strcmp(ext, ".fnt") == 0 || strcmp(ext, ".bin") == 0)) {
                                    // Save as raw binary (no header)
                                    success = SaveVGAFontRaw(&g_font, filename);
                                    formatName = "Raw Binary";
                                } else {
                                    // Save as VGAF format (with header)
                                    success = SaveVGAFont(&g_font, filename);
                                    formatName = "VGAF (VGA Font Editor)";
                                }
                                
                                if (success) {
                                    char msg[512];
                                    sprintf(msg, "Font saved successfully!\n\n"
                                               "File: %s\n"
                                               "Size: %ld bytes\n"
                                               "Format: %s",
                                               filename, GetFontFileSize(filename), formatName);
                                    MessageBox(hwnd, msg, "Save Complete", MB_OK | MB_ICONINFORMATION);
                                    
                                    // Update status bar
                                    if (g_hStatusBar) {
                                        SendMessage(g_hStatusBar, SB_SETTEXT, 3, (LPARAM)"Font saved");
                                    }
                                } else {
                                    MessageBox(hwnd, "Failed to save font file", "Save Error", MB_OK | MB_ICONERROR);
                                }
                            }
                        }
                    }
                    break;
                    
                case ID_FILE_EXIT:
                    PostQuitMessage(0);
                    break;
                    
                // Edit menu commands
                case ID_EDIT_COPY:
                    CopyCharacterToClipboard(g_selectedChar);
                    UpdateStatusBar();
                    break;
                    
                case ID_EDIT_PASTE:
                    PasteCharacterFromClipboard(g_selectedChar);
                    InvalidateRect(g_hCharGrid, NULL, FALSE);
                    InvalidateRect(g_hPixelEditor, NULL, FALSE);
                    UpdateStatusBar();
                    break;
                    
                case ID_EDIT_CLEAR:
                    ClearCharacter(&g_font, g_selectedChar);
                    InvalidateRect(g_hCharGrid, NULL, FALSE);
                    InvalidateRect(g_hPixelEditor, NULL, FALSE);
                    UpdateStatusBar();
                    break;
                    
                case ID_HELP_ABOUT:
                    MessageBox(hwnd, 
                        "VGA Font Editor v0.1.0\n\n"
                        "Copyright (c) 2025 Erdem Ersoy (eersoy93)\n\n"
                        "A simple editor for VGA text mode fonts (8x16 pixels)\n\n"
                        "Usage:\n"
                        "- Click on a character in the grid to select it\n"
                        "- Edit the character in the pixel editor on the right\n"
                        "- Left click to set pixels, right click to clear pixels",
                        "About VGA Font Editor", 
                        MB_OK | MB_ICONINFORMATION);
                    break;
                    
                // Toolbar komutları
                case ID_TB_ZOOM_IN:
                    if (g_zoomLevel < MAX_ZOOM) {
                        g_zoomLevel++;
                        InvalidateRect(g_hPixelEditor, NULL, TRUE);
                        UpdateStatusBar();
                    }
                    break;
                    
                case ID_TB_ZOOM_OUT:
                    if (g_zoomLevel > MIN_ZOOM) {
                        g_zoomLevel--;
                        InvalidateRect(g_hPixelEditor, NULL, TRUE);
                        UpdateStatusBar();
                    }
                    break;
            }
            break;
            
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_F1:
                    SendMessage(hwnd, WM_COMMAND, ID_HELP_ABOUT, 0);
                    break;
                case VK_DELETE:
                    SendMessage(hwnd, WM_COMMAND, ID_EDIT_CLEAR, 0);
                    break;
            }
            break;
            
        case WM_CHAR:
            // Handle Ctrl key combinations
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                switch (wParam) {
                    case 'o': case 'O':
                        SendMessage(hwnd, WM_COMMAND, ID_FILE_OPEN, 0);
                        break;
                    case 's': case 'S':
                        SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0);
                        break;
                    case 'c': case 'C':
                        SendMessage(hwnd, WM_COMMAND, ID_EDIT_COPY, 0);
                        break;
                    case 'v': case 'V':
                        SendMessage(hwnd, WM_COMMAND, ID_EDIT_PASTE, 0);
                        break;
                    case 'z': case 'Z':
                        SendMessage(hwnd, WM_COMMAND, ID_EDIT_UNDO, 0);
                        break;
                    case 'y': case 'Y':
                        SendMessage(hwnd, WM_COMMAND, ID_EDIT_REDO, 0);
                        break;
                }
            }
            break;
            
        case WM_SIZE:
            if (g_hCharGrid && g_hPixelEditor && g_hCharInfo && g_hToolbar && g_hStatusBar) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                // Resize toolbar and status bar
                SendMessage(g_hToolbar, TB_AUTOSIZE, 0, 0);
                SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
                
                // Calculate available space
                RECT tbRect, sbRect;
                GetWindowRect(g_hToolbar, &tbRect);
                GetWindowRect(g_hStatusBar, &sbRect);
                
                int toolbarHeight = tbRect.bottom - tbRect.top;
                int statusbarHeight = sbRect.bottom - sbRect.top;
                int availableHeight = rect.bottom - toolbarHeight - statusbarHeight;
                int availableWidth = rect.right;
                
                // Safety check - minimum window size
                if (availableWidth < 600 || availableHeight < 400) {
                    break; // Skip layout if window is too small
                }
                
                // Modern layout calculations with bounds checking
                int margin = 10; // Reduced margin for smaller windows
                int spacing = 15; // Reduced spacing
                
                // Calculate grid size - ensure it fits
                int maxGridWidth = (availableWidth * 55) / 100; // 55% of width for grid
                int maxGridHeight = availableHeight - (2 * margin);
                int gridSize = min(maxGridWidth, maxGridHeight);
                gridSize = max(gridSize, 200); // Minimum size but smaller than before
                gridSize = min(gridSize, 600); // Maximum size to prevent overflow
                
                // Calculate editor size - ensure it fits in remaining space
                int remainingWidth = availableWidth - gridSize - (2 * margin) - spacing;
                int maxEditorHeight = availableHeight - margin - 100; // Leave space for info panel
                int editorSize = min(remainingWidth, maxEditorHeight);
                editorSize = max(editorSize, 150); // Minimum editor size
                editorSize = min(editorSize, 500); // Maximum editor size
                
                // Ensure controls don't exceed window bounds
                int rightEdge = margin + gridSize + spacing + editorSize;
                if (rightEdge > availableWidth) {
                    // Adjust sizes if they don't fit
                    int excessWidth = rightEdge - availableWidth + 10;
                    if (gridSize > editorSize) {
                        gridSize -= excessWidth / 2;
                        editorSize -= excessWidth / 2;
                    } else {
                        editorSize -= excessWidth / 2;
                        gridSize -= excessWidth / 2;
                    }
                    // Ensure minimum sizes
                    gridSize = max(gridSize, 200);
                    editorSize = max(editorSize, 150);
                }
                
                // Position controls with bounds checking
                MoveWindow(g_hCharGrid, 
                          margin, 
                          toolbarHeight + margin, 
                          gridSize, 
                          gridSize, 
                          TRUE);
                          
                MoveWindow(g_hPixelEditor, 
                          margin + gridSize + spacing, 
                          toolbarHeight + margin, 
                          editorSize, 
                          editorSize, 
                          TRUE);
                          
                // Info panel - ensure it fits
                int infoPanelY = toolbarHeight + margin + editorSize + 10;
                int infoPanelHeight = availableHeight - (infoPanelY - toolbarHeight) - margin;
                infoPanelHeight = max(infoPanelHeight, 80); // Minimum height
                infoPanelHeight = min(infoPanelHeight, 120); // Maximum height
                
                MoveWindow(g_hCharInfo, 
                          margin + gridSize + spacing, 
                          infoPanelY, 
                          editorSize, 
                          infoPanelHeight, 
                          TRUE);
            }
            break;
            
        case WM_THEMECHANGED:
            // Handle theme changes (e.g., when user switches between classic and themed)
            if (g_hTheme) {
                CloseThemeData(g_hTheme);
                g_hTheme = NULL;
            }
            
            // Reinitialize theming
            InitializeTheming();
            
            if (IsThemingEnabled()) {
                g_hTheme = OpenThemeData(hwnd, L"BUTTON");
                
                // Reapply theming to all controls
                ApplyModernStyling(g_hCharGrid);
                ApplyModernStyling(g_hPixelEditor);
                ApplyModernStyling(g_hCharInfo);
                ApplyModernStyling(g_hToolbar);
                ApplyModernStyling(g_hStatusBar);
            }
            
            // Redraw everything
            InvalidateRect(hwnd, NULL, TRUE);
            break;
            
        case WM_DWMCOMPOSITIONCHANGED:
            // Handle DWM composition changes (Windows Vista+)
            InitializeTheming();
            InvalidateRect(hwnd, NULL, TRUE);
            break;
            
        case WM_DESTROY:
            // Clean up theme data
            if (g_hTheme) {
                CloseThemeData(g_hTheme);
                g_hTheme = NULL;
            }
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void CreateControls(HWND hwnd)
{
    // Create toolbar with modern styling
    CreateToolbar(hwnd);
    
    // Create status bar with modern styling
    CreateStatusBar(hwnd);
    
    // Create character grid with conservative initial size and position
    g_hCharGrid = CreateWindowEx(
        WS_EX_CLIENTEDGE, // Modern 3D border
        "CharGrid", "",
        WS_CHILD | WS_VISIBLE,
        10, TOOLBAR_HEIGHT + 10, 400, 400, // Conservative initial size
        hwnd, NULL, g_hInstance, NULL
    );
    
    // Create pixel editor with conservative initial size and position
    g_hPixelEditor = CreateWindowEx(
        WS_EX_CLIENTEDGE, // Modern 3D border
        "PixelEditor", "",
        WS_CHILD | WS_VISIBLE,
        420, TOOLBAR_HEIGHT + 10, 300, 300, // Conservative initial size
        hwnd, NULL, g_hInstance, NULL
    );
    
    // Create character info with conservative initial size
    g_hCharInfo = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "STATIC", "",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_SUNKEN,
        420, TOOLBAR_HEIGHT + 320, 300, 100, // Conservative initial size
        hwnd, NULL, g_hInstance, NULL
    );
    
    // Apply modern theming to controls
    if (IsThemingEnabled()) {
        ApplyModernStyling(g_hCharGrid);
        ApplyModernStyling(g_hPixelEditor);
        ApplyModernStyling(g_hCharInfo);
        ApplyModernStyling(g_hToolbar);
        ApplyModernStyling(g_hStatusBar);
        
        // Open theme for button styling
        if (!g_hTheme) {
            g_hTheme = OpenThemeData(hwnd, L"BUTTON");
        }
    }
    
    // Modern font settings for character info
    if (g_hCharInfo) {
        HFONT hFont = CreateFont(
            12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_SWISS, "Segoe UI"
        );
        SendMessage(g_hCharInfo, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    
    UpdateCharacterInfo();
    UpdateStatusBar();
}

void LoadDefaultFont()
{
    // Initialize with a basic font (could load from system or embedded data)
    InitializeVGAFont(&g_font);
    
    // Load some basic ASCII characters as examples
    LoadBasicASCIIChars(&g_font);
}

void UpdateCharacterInfo()
{
    if (g_hCharInfo) {
        char info[512];
        sprintf(info, 
            "Selected Character: %d (0x%02X) - '%c'\r\n"
            "Position: Row %d, Column %d\r\n"
            "Zoom: %dx\r\n"
            "\r\n"
            "Usage:\r\n"
            "• Left click: Set pixel\r\n"
            "• Right click: Clear pixel\r\n"
            "• Ctrl+C: Copy, Ctrl+V: Paste\r\n"
            "• Delete: Clear character",
            g_selectedChar,
            g_selectedChar,
            (g_selectedChar >= 32 && g_selectedChar <= 126) ? g_selectedChar : '?',
            g_selectedChar / 16,
            g_selectedChar % 16,
            g_zoomLevel
        );
        SetWindowText(g_hCharInfo, info);
    }
}

void CreateToolbar(HWND hwnd)
{
    // Modern toolbar oluştur - sadece metin tabanlı
    g_hToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | TBSTYLE_LIST | CCS_TOP,
        0, 0, 0, 0, hwnd, (HMENU)ID_TOOLBAR, g_hInstance, NULL);
    
    if (g_hToolbar) {
        // Modern toolbar ayarları - simgesiz tasarım
        SendMessage(g_hToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
        SendMessage(g_hToolbar, TB_SETBITMAPSIZE, 0, MAKELONG(0, 0)); // Hiç simge yok
        SendMessage(g_hToolbar, TB_SETBUTTONSIZE, 0, MAKELONG(60, 28)); // Metin için uygun boyut
        
        // Sadece metin tabanlı toolbar butonları
        TBBUTTON tbButtons[] = {
            {I_IMAGENONE, ID_FILE_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Open"},
            {I_IMAGENONE, ID_FILE_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Save"},
            {I_IMAGENONE, 0, 0, TBSTYLE_SEP, {0}, 0, 0}, // Ayırıcı
            {I_IMAGENONE, ID_EDIT_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Copy"},
            {I_IMAGENONE, ID_EDIT_PASTE, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Paste"},
            {I_IMAGENONE, ID_EDIT_CLEAR, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Clear"},
            {I_IMAGENONE, 0, 0, TBSTYLE_SEP, {0}, 0, 0}, // Ayırıcı
            {I_IMAGENONE, ID_TB_ZOOM_OUT, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Zoom -"},
            {I_IMAGENONE, ID_TB_ZOOM_IN, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)"Zoom +"}
        };
        
        SendMessage(g_hToolbar, TB_ADDBUTTONS, sizeof(tbButtons)/sizeof(TBBUTTON), (LPARAM)&tbButtons);
        SendMessage(g_hToolbar, TB_AUTOSIZE, 0, 0);
    }
}

void CreateStatusBar(HWND hwnd)
{
    g_hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd, (HMENU)ID_STATUSBAR, g_hInstance, NULL);
    
    if (g_hStatusBar) {
        // Create status bar parts
        int parts[] = {150, 300, 450, -1};
        SendMessage(g_hStatusBar, SB_SETPARTS, 4, (LPARAM)parts);
    }
}

void UpdateStatusBar()
{
    if (g_hStatusBar) {
        char text[256];
        
        // Part 0: Character info
        sprintf(text, "Character: %d (0x%02X)", g_selectedChar, g_selectedChar);
        SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)text);
        
        // Part 1: Position
        sprintf(text, "Row: %d, Column: %d", g_selectedChar / 16, g_selectedChar % 16);
        SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)text);
        
        // Part 2: Zoom level
        sprintf(text, "Zoom: %dx", g_zoomLevel);
        SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)text);
        
        // Part 3: Ready status
        SendMessage(g_hStatusBar, SB_SETTEXT, 3, (LPARAM)"Ready");
    }
}

void CopyCharacterToClipboard(int charIndex)
{
    if (charIndex >= 0 && charIndex < VGA_FONT_CHARS) {
        for (int i = 0; i < VGA_CHAR_HEIGHT; i++) {
            g_clipboard.data[i] = g_font.data[charIndex][i];
        }
        g_clipboard.hasData = TRUE;
        
        // Update status
        if (g_hStatusBar) {
            SendMessage(g_hStatusBar, SB_SETTEXT, 3, (LPARAM)"Character copied to clipboard");
        }
    }
}

void PasteCharacterFromClipboard(int charIndex)
{
    if (g_clipboard.hasData && charIndex >= 0 && charIndex < VGA_FONT_CHARS) {
        for (int i = 0; i < VGA_CHAR_HEIGHT; i++) {
            g_font.data[charIndex][i] = g_clipboard.data[i];
        }
        
        // Update status
        if (g_hStatusBar) {
            SendMessage(g_hStatusBar, SB_SETTEXT, 3, (LPARAM)"Character pasted from clipboard");
        }
    }
}

// UXTHEME support functions
void InitializeTheming()
{
    // Initialize theme support
    g_bThemingEnabled = IsAppThemed() && IsThemeActive();
    
    if (g_bThemingEnabled) {
        // Enable DWM composition for better visual effects (Windows Vista+)
        BOOL enabled = FALSE;
        if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled) {
            // Enable glass effect on window - only if main window exists
            if (g_hMainWindow) {
                MARGINS margins = {-1, -1, -1, -1};
                DwmExtendFrameIntoClientArea(g_hMainWindow, &margins);
            }
        }
    }
}

void EnableVisualStyles()
{
    // Enable visual styles by calling SetWindowTheme
    // This must be called before creating windows
    if (IsThemeActive()) {
        g_bThemingEnabled = TRUE;
    }
}

BOOL IsThemingEnabled()
{
    return g_bThemingEnabled && IsAppThemed() && IsThemeActive();
}

void ApplyModernStyling(HWND hwnd)
{
    if (!IsThemingEnabled()) {
        return;
    }
    
    // Apply modern theme to specific controls
    SetWindowTheme(hwnd, L"Explorer", NULL);
    
    // Enable double buffering to reduce flicker
    SetWindowLong(hwnd, GWL_EXSTYLE, 
                  GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_COMPOSITED);
}

void DrawThemedButton(HDC hdc, RECT* rect, int state, const char* text)
{
    if (!IsThemingEnabled() || !g_hTheme) {
        // Fallback to classic drawing
        DrawEdge(hdc, rect, EDGE_RAISED, BF_RECT);
        DrawText(hdc, text, -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return;
    }
    
    // Use themed drawing
    int themeState = PBS_NORMAL;
    switch (state) {
        case 1: themeState = PBS_HOT; break;
        case 2: themeState = PBS_PRESSED; break;
        case 3: themeState = PBS_DISABLED; break;
    }
    
    DrawThemeBackground(g_hTheme, hdc, BP_PUSHBUTTON, themeState, rect, NULL);
    
    // Draw text with theme
    RECT textRect = *rect;
    DrawThemeText(g_hTheme, hdc, BP_PUSHBUTTON, themeState, 
                  (LPCWSTR)text, -1, DT_CENTER | DT_VCENTER | DT_SINGLELINE, 
                  0, &textRect);
}

void DrawThemedBackground(HDC hdc, RECT* rect, int partId, int stateId)
{
    if (!IsThemingEnabled() || !g_hTheme) {
        // Fallback to classic drawing
        FillRect(hdc, rect, GetSysColorBrush(COLOR_BTNFACE));
        return;
    }
    
    // Use themed background drawing
    DrawThemeBackground(g_hTheme, hdc, partId, stateId, rect, NULL);
}
