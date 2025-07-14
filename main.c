#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vga_font_editor.h"

// Global variables
HINSTANCE g_hInstance;
HWND g_hMainWindow;
VGAFont g_font;
int g_selectedChar = 0;
HWND g_hCharGrid, g_hPixelEditor, g_hCharInfo;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CharGridProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PixelEditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd);
void LoadDefaultFont();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Suppress unused parameter warnings
    (void)hPrevInstance;
    (void)lpCmdLine;
    
    g_hInstance = hInstance;
    
    // Register window classes
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "VGAFontEditor";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
    RegisterClass(&wc);
    
    // Register custom control classes
    wc.lpfnWndProc = CharGridProc;
    wc.lpszClassName = "CharGrid";
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    RegisterClass(&wc);
    
    wc.lpfnWndProc = PixelEditorProc;
    wc.lpszClassName = "PixelEditor";
    RegisterClass(&wc);
    
    // Create main window
    g_hMainWindow = CreateWindow(
        "VGAFontEditor",
        "VGA Font Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 700,
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
                        ofn.lpstrFilter = "VGA Font Files\0*.fnt\0All Files\0*.*\0";
                        ofn.nFilterIndex = 1;
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                        
                        if (GetOpenFileName(&ofn)) {
                            if (LoadVGAFont(&g_font, filename)) {
                                InvalidateRect(g_hCharGrid, NULL, TRUE);
                                InvalidateRect(g_hPixelEditor, NULL, TRUE);
                            } else {
                                MessageBox(hwnd, "Failed to load font file", "Error", MB_OK | MB_ICONERROR);
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
                        ofn.lpstrFilter = "VGA Font Files\0*.fnt\0All Files\0*.*\0";
                        ofn.nFilterIndex = 1;
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
                        
                        if (GetSaveFileName(&ofn)) {
                            if (!SaveVGAFont(&g_font, filename)) {
                                MessageBox(hwnd, "Failed to save font file", "Error", MB_OK | MB_ICONERROR);
                            }
                        }
                    }
                    break;
                    
                case ID_FILE_EXIT:
                    PostQuitMessage(0);
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
            }
            break;
            
        case WM_SIZE:
            if (g_hCharGrid && g_hPixelEditor && g_hCharInfo) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                // Resize controls
                MoveWindow(g_hCharGrid, 10, 10, 520, 520, TRUE);
                MoveWindow(g_hPixelEditor, 550, 10, 400, 400, TRUE);
                MoveWindow(g_hCharInfo, 550, 420, 400, 100, TRUE);
            }
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void CreateControls(HWND hwnd)
{
    // Create character grid
    g_hCharGrid = CreateWindow(
        "CharGrid", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        10, 10, 520, 520,
        hwnd, NULL, g_hInstance, NULL
    );
    
    // Create pixel editor
    g_hPixelEditor = CreateWindow(
        "PixelEditor", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        550, 10, 400, 400,
        hwnd, NULL, g_hInstance, NULL
    );
    
    // Create character info static text
    g_hCharInfo = CreateWindow(
        "STATIC", "",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        550, 420, 400, 100,
        hwnd, NULL, g_hInstance, NULL
    );
    
    UpdateCharacterInfo();
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
        char info[256];
        sprintf(info, 
            "Selected Character: %d (0x%02X)\n"
            "ASCII: '%c'\n"
            "Position: Row %d, Column %d",
            g_selectedChar,
            g_selectedChar,
            (g_selectedChar >= 32 && g_selectedChar <= 126) ? g_selectedChar : '?',
            g_selectedChar / 16,
            g_selectedChar % 16
        );
        SetWindowText(g_hCharInfo, info);
    }
}
