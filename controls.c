#include "vga_font_editor.h"

// Character Grid Control
LRESULT CALLBACK CharGridProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL isDragging = FALSE;
    
    switch (uMsg) {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                // Fill background
                FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
                
                // Calculate cell size
                int cellWidth = (rect.right - rect.left) / 16;
                int cellHeight = (rect.bottom - rect.top) / 16;
                
                // Draw grid lines
                HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
                HPEN oldPen = SelectObject(hdc, gridPen);
                
                // Vertical lines
                for (int x = 0; x <= 16; x++) {
                    int xPos = x * cellWidth;
                    MoveToEx(hdc, xPos, 0, NULL);
                    LineTo(hdc, xPos, rect.bottom);
                }
                
                // Horizontal lines
                for (int y = 0; y <= 16; y++) {
                    int yPos = y * cellHeight;
                    MoveToEx(hdc, 0, yPos, NULL);
                    LineTo(hdc, rect.right, yPos);
                }
                
                // Draw characters
                HFONT font = CreateFont(
                    min(cellHeight - 4, cellWidth - 4), 0, 0, 0,
                    FW_NORMAL, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                    FIXED_PITCH | FF_MODERN, "Courier New"
                );
                HFONT oldFont = SelectObject(hdc, font);
                
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(0, 0, 0));
                
                for (int row = 0; row < 16; row++) {
                    for (int col = 0; col < 16; col++) {
                        int charIndex = row * 16 + col;
                        
                        // Highlight selected character
                        if (charIndex == g_selectedChar) {
                            RECT cellRect = {
                                col * cellWidth + 1,
                                row * cellHeight + 1,
                                (col + 1) * cellWidth - 1,
                                (row + 1) * cellHeight - 1
                            };
                            FillRect(hdc, &cellRect, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
                        }
                        
                        // Draw character using VGA font data
                        DrawVGACharacter(hdc, 
                                       col * cellWidth + 2, 
                                       row * cellHeight + 2,
                                       cellWidth - 4, 
                                       cellHeight - 4,
                                       &g_font, charIndex);
                    }
                }
                
                SelectObject(hdc, oldFont);
                DeleteObject(font);
                SelectObject(hdc, oldPen);
                DeleteObject(gridPen);
                
                EndPaint(hwnd, &ps);
            }
            break;
            
        case WM_LBUTTONDOWN:
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                int cellWidth = (rect.right - rect.left) / 16;
                int cellHeight = (rect.bottom - rect.top) / 16;
                
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                
                int col = x / cellWidth;
                int row = y / cellHeight;
                
                if (col >= 0 && col < 16 && row >= 0 && row < 16) {
                    int newSelected = row * 16 + col;
                    if (newSelected != g_selectedChar) {
                        g_selectedChar = newSelected;
                        UpdateCharacterInfo();
                        InvalidateRect(hwnd, NULL, FALSE);
                        InvalidateRect(g_hPixelEditor, NULL, FALSE);
                    }
                }
                
                SetCapture(hwnd);
                isDragging = TRUE;
            }
            break;
            
        case WM_MOUSEMOVE:
            if (isDragging && (wParam & MK_LBUTTON)) {
                // Handle dragging to select different characters
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                int cellWidth = (rect.right - rect.left) / 16;
                int cellHeight = (rect.bottom - rect.top) / 16;
                
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                
                int col = x / cellWidth;
                int row = y / cellHeight;
                
                if (col >= 0 && col < 16 && row >= 0 && row < 16) {
                    int newSelected = row * 16 + col;
                    if (newSelected != g_selectedChar) {
                        g_selectedChar = newSelected;
                        UpdateCharacterInfo();
                        InvalidateRect(hwnd, NULL, FALSE);
                        InvalidateRect(g_hPixelEditor, NULL, FALSE);
                    }
                }
            }
            break;
            
        case WM_LBUTTONUP:
            if (isDragging) {
                ReleaseCapture();
                isDragging = FALSE;
            }
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Pixel Editor Control
LRESULT CALLBACK PixelEditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL isDragging = FALSE;
    static BOOL drawMode = TRUE; // TRUE = set pixels, FALSE = clear pixels
    
    switch (uMsg) {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                // Fill background
                FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
                
                // Calculate pixel size
                int pixelWidth = (rect.right - rect.left) / VGA_CHAR_WIDTH;
                int pixelHeight = (rect.bottom - rect.top) / VGA_CHAR_HEIGHT;
                
                // Draw grid lines
                HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
                HPEN oldPen = SelectObject(hdc, gridPen);
                
                // Vertical lines
                for (int x = 0; x <= VGA_CHAR_WIDTH; x++) {
                    int xPos = x * pixelWidth;
                    MoveToEx(hdc, xPos, 0, NULL);
                    LineTo(hdc, xPos, rect.bottom);
                }
                
                // Horizontal lines
                for (int y = 0; y <= VGA_CHAR_HEIGHT; y++) {
                    int yPos = y * pixelHeight;
                    MoveToEx(hdc, 0, yPos, NULL);
                    LineTo(hdc, rect.right, yPos);
                }
                
                // Draw pixels
                HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
                
                for (int y = 0; y < VGA_CHAR_HEIGHT; y++) {
                    for (int x = 0; x < VGA_CHAR_WIDTH; x++) {
                        if (GetFontPixel(&g_font, g_selectedChar, x, y)) {
                            RECT pixelRect = {
                                x * pixelWidth + 1,
                                y * pixelHeight + 1,
                                (x + 1) * pixelWidth - 1,
                                (y + 1) * pixelHeight - 1
                            };
                            FillRect(hdc, &pixelRect, blackBrush);
                        }
                    }
                }
                
                SelectObject(hdc, oldPen);
                DeleteObject(gridPen);
                
                EndPaint(hwnd, &ps);
            }
            break;
            
        case WM_LBUTTONDOWN:
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                int pixelWidth = (rect.right - rect.left) / VGA_CHAR_WIDTH;
                int pixelHeight = (rect.bottom - rect.top) / VGA_CHAR_HEIGHT;
                
                int x = LOWORD(lParam) / pixelWidth;
                int y = HIWORD(lParam) / pixelHeight;
                
                if (x >= 0 && x < VGA_CHAR_WIDTH && y >= 0 && y < VGA_CHAR_HEIGHT) {
                    drawMode = TRUE;
                    SetFontPixel(&g_font, g_selectedChar, x, y, TRUE);
                    InvalidateRect(hwnd, NULL, FALSE);
                    InvalidateRect(g_hCharGrid, NULL, FALSE);
                }
                
                SetCapture(hwnd);
                isDragging = TRUE;
            }
            break;
            
        case WM_RBUTTONDOWN:
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                int pixelWidth = (rect.right - rect.left) / VGA_CHAR_WIDTH;
                int pixelHeight = (rect.bottom - rect.top) / VGA_CHAR_HEIGHT;
                
                int x = LOWORD(lParam) / pixelWidth;
                int y = HIWORD(lParam) / pixelHeight;
                
                if (x >= 0 && x < VGA_CHAR_WIDTH && y >= 0 && y < VGA_CHAR_HEIGHT) {
                    drawMode = FALSE;
                    SetFontPixel(&g_font, g_selectedChar, x, y, FALSE);
                    InvalidateRect(hwnd, NULL, FALSE);
                    InvalidateRect(g_hCharGrid, NULL, FALSE);
                }
                
                SetCapture(hwnd);
                isDragging = TRUE;
            }
            break;
            
        case WM_MOUSEMOVE:
            if (isDragging && ((wParam & MK_LBUTTON) || (wParam & MK_RBUTTON))) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                int pixelWidth = (rect.right - rect.left) / VGA_CHAR_WIDTH;
                int pixelHeight = (rect.bottom - rect.top) / VGA_CHAR_HEIGHT;
                
                int x = LOWORD(lParam) / pixelWidth;
                int y = HIWORD(lParam) / pixelHeight;
                
                if (x >= 0 && x < VGA_CHAR_WIDTH && y >= 0 && y < VGA_CHAR_HEIGHT) {
                    BOOL currentPixel = GetFontPixel(&g_font, g_selectedChar, x, y);
                    if (currentPixel != drawMode) {
                        SetFontPixel(&g_font, g_selectedChar, x, y, drawMode);
                        InvalidateRect(hwnd, NULL, FALSE);
                        InvalidateRect(g_hCharGrid, NULL, FALSE);
                    }
                }
            }
            break;
            
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            if (isDragging) {
                ReleaseCapture();
                isDragging = FALSE;
            }
            break;
            
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_DELETE:
                case VK_BACK:
                    // Clear the current character
                    ClearCharacter(&g_font, g_selectedChar);
                    InvalidateRect(hwnd, NULL, FALSE);
                    InvalidateRect(g_hCharGrid, NULL, FALSE);
                    break;
            }
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Helper function to draw VGA character
void DrawVGACharacter(HDC hdc, int x, int y, int width, int height, VGAFont* font, int charIndex)
{
    if (!font || charIndex < 0 || charIndex >= VGA_FONT_CHARS) return;
    
    // Calculate pixel size
    int pixelWidth = width / VGA_CHAR_WIDTH;
    int pixelHeight = height / VGA_CHAR_HEIGHT;
    
    if (pixelWidth < 1) pixelWidth = 1;
    if (pixelHeight < 1) pixelHeight = 1;
    
    HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    for (int py = 0; py < VGA_CHAR_HEIGHT; py++) {
        for (int px = 0; px < VGA_CHAR_WIDTH; px++) {
            if (GetFontPixel(font, charIndex, px, py)) {
                RECT pixelRect = {
                    x + px * pixelWidth,
                    y + py * pixelHeight,
                    x + (px + 1) * pixelWidth,
                    y + (py + 1) * pixelHeight
                };
                FillRect(hdc, &pixelRect, blackBrush);
            }
        }
    }
}
