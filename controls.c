#include "vga_font_editor.h"

// Character Grid Control - Modern UI ile güncellendi
LRESULT CALLBACK CharGridProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL isDragging = FALSE;
    static int hoverChar = -1;
    
    switch (uMsg) {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                // Modern arka plan rengi
                HBRUSH bgBrush = CreateSolidBrush(COLOR_MODERN_BG);
                FillRect(hdc, &rect, bgBrush);
                DeleteObject(bgBrush);
                
                // Calculate cell size
                int cellWidth = (rect.right - rect.left) / 16;
                int cellHeight = (rect.bottom - rect.top) / 16;
                
                // Modern grid çizgileri - daha ince ve yumuşak
                HPEN gridPen = CreatePen(PS_SOLID, 1, COLOR_MODERN_GRID);
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
                
                // Draw characters with modern styling
                HFONT font = CreateFont(
                    min(cellHeight - 6, cellWidth - 6), 0, 0, 0,
                    FW_NORMAL, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                    FIXED_PITCH | FF_MODERN, "Consolas"
                );
                HFONT oldFont = SelectObject(hdc, font);
                
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, COLOR_MODERN_TEXT);
                
                for (int row = 0; row < 16; row++) {
                    for (int col = 0; col < 16; col++) {
                        int charIndex = row * 16 + col;
                        
                        RECT cellRect = {
                            col * cellWidth + 1,
                            row * cellHeight + 1,
                            (col + 1) * cellWidth - 1,
                            (row + 1) * cellHeight - 1
                        };
                        
                        // Modern hover efekti
                        if (charIndex == hoverChar && charIndex != g_selectedChar) {
                            HBRUSH hoverBrush = CreateSolidBrush(COLOR_MODERN_HOVER);
                            FillRect(hdc, &cellRect, hoverBrush);
                            DeleteObject(hoverBrush);
                        }
                        
                        // Modern seçim vurgusu - yumuşak mavi
                        if (charIndex == g_selectedChar) {
                            HBRUSH selectedBrush = CreateSolidBrush(COLOR_MODERN_SELECTED);
                            FillRect(hdc, &cellRect, selectedBrush);
                            DeleteObject(selectedBrush);
                            
                            // Seçim kenarlığı
                            HPEN borderPen = CreatePen(PS_SOLID, 2, COLOR_MODERN_HIGHLIGHT);
                            HPEN oldBorderPen = SelectObject(hdc, borderPen);
                            HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                            HBRUSH oldBrush = SelectObject(hdc, nullBrush);
                            
                            Rectangle(hdc, cellRect.left, cellRect.top, cellRect.right, cellRect.bottom);
                            
                            SelectObject(hdc, oldBrush);
                            SelectObject(hdc, oldBorderPen);
                            DeleteObject(borderPen);
                        }
                        
                        // Draw character using VGA font data with better positioning
                        DrawVGACharacter(hdc, 
                                       col * cellWidth + 3, 
                                       row * cellHeight + 3,
                                       cellWidth - 6, 
                                       cellHeight - 6,
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
            
        case WM_MOUSEMOVE:
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                int cellWidth = (rect.right - rect.left) / 16;
                int cellHeight = (rect.bottom - rect.top) / 16;
                
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                
                int col = x / cellWidth;
                int row = y / cellHeight;
                
                int newHover = -1;
                if (col >= 0 && col < 16 && row >= 0 && row < 16) {
                    newHover = row * 16 + col;
                }
                
                if (newHover != hoverChar) {
                    hoverChar = newHover;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                
                if (isDragging && (wParam & MK_LBUTTON)) {
                    // Handle dragging to select different characters
                    if (col >= 0 && col < 16 && row >= 0 && row < 16) {
                        int newSelected = row * 16 + col;
                        if (newSelected != g_selectedChar) {
                            g_selectedChar = newSelected;
                            UpdateCharacterInfo();
                            UpdateStatusBar();
                            InvalidateRect(hwnd, NULL, FALSE);
                            InvalidateRect(g_hPixelEditor, NULL, FALSE);
                        }
                    }
                }
            }
            break;
            
        case WM_MOUSELEAVE:
            if (hoverChar != -1) {
                hoverChar = -1;
                InvalidateRect(hwnd, NULL, FALSE);
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
                        UpdateStatusBar();
                        InvalidateRect(hwnd, NULL, FALSE);
                        InvalidateRect(g_hPixelEditor, NULL, FALSE);
                    }
                }
                
                SetCapture(hwnd);
                isDragging = TRUE;
                
                // Mouse tracking için
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);
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

// Pixel Editor Control - Modern UI ile güncellendi
LRESULT CALLBACK PixelEditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL isDragging = FALSE;
    static BOOL drawMode = TRUE; // TRUE = set pixels, FALSE = clear pixels
    static int hoverX = -1, hoverY = -1;
    
    switch (uMsg) {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                // Modern arka plan
                HBRUSH bgBrush = CreateSolidBrush(COLOR_MODERN_WHITE);
                FillRect(hdc, &rect, bgBrush);
                DeleteObject(bgBrush);
                
                // Calculate pixel size with zoom support
                int basePixelWidth = (rect.right - rect.left) / VGA_CHAR_WIDTH;
                int basePixelHeight = (rect.bottom - rect.top) / VGA_CHAR_HEIGHT;
                int pixelWidth = (basePixelWidth * g_zoomLevel) / 2;
                int pixelHeight = (basePixelHeight * g_zoomLevel) / 2;
                
                // Modern grid çizgileri - daha ince
                HPEN gridPen = CreatePen(PS_SOLID, 1, COLOR_MODERN_BORDER);
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
                
                // Draw pixels with modern styling
                for (int y = 0; y < VGA_CHAR_HEIGHT; y++) {
                    for (int x = 0; x < VGA_CHAR_WIDTH; x++) {
                        RECT pixelRect = {
                            x * pixelWidth + 1,
                            y * pixelHeight + 1,
                            (x + 1) * pixelWidth - 1,
                            (y + 1) * pixelHeight - 1
                        };
                        
                        // Hover efekti
                        if (x == hoverX && y == hoverY) {
                            HBRUSH hoverBrush = CreateSolidBrush(COLOR_MODERN_HOVER);
                            FillRect(hdc, &pixelRect, hoverBrush);
                            DeleteObject(hoverBrush);
                        }
                        
                        // Aktif piksel - modern koyu renk
                        if (GetFontPixel(&g_font, g_selectedChar, x, y)) {
                            HBRUSH pixelBrush = CreateSolidBrush(COLOR_MODERN_PIXEL);
                            FillRect(hdc, &pixelRect, pixelBrush);
                            DeleteObject(pixelBrush);
                        }
                    }
                }
                
                SelectObject(hdc, oldPen);
                DeleteObject(gridPen);
                
                EndPaint(hwnd, &ps);
            }
            break;
            
        case WM_MOUSEMOVE:
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                int basePixelWidth = (rect.right - rect.left) / VGA_CHAR_WIDTH;
                int basePixelHeight = (rect.bottom - rect.top) / VGA_CHAR_HEIGHT;
                int pixelWidth = (basePixelWidth * g_zoomLevel) / 2;
                int pixelHeight = (basePixelHeight * g_zoomLevel) / 2;
                
                int x = LOWORD(lParam) / pixelWidth;
                int y = HIWORD(lParam) / pixelHeight;
                
                int newHoverX = -1, newHoverY = -1;
                if (x >= 0 && x < VGA_CHAR_WIDTH && y >= 0 && y < VGA_CHAR_HEIGHT) {
                    newHoverX = x;
                    newHoverY = y;
                }
                
                if (newHoverX != hoverX || newHoverY != hoverY) {
                    hoverX = newHoverX;
                    hoverY = newHoverY;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                
                if (isDragging && ((wParam & MK_LBUTTON) || (wParam & MK_RBUTTON))) {
                    if (x >= 0 && x < VGA_CHAR_WIDTH && y >= 0 && y < VGA_CHAR_HEIGHT) {
                        BOOL currentPixel = GetFontPixel(&g_font, g_selectedChar, x, y);
                        if (currentPixel != drawMode) {
                            // Add undo action for pixel change during drag
                            AddUndoAction(ACTION_PIXEL_CHANGE, g_selectedChar, x, y, currentPixel, drawMode);
                            SetFontPixel(&g_font, g_selectedChar, x, y, drawMode);
                            InvalidateRect(hwnd, NULL, FALSE);
                            InvalidateRect(g_hCharGrid, NULL, FALSE);
                        }
                    }
                }
            }
            break;
            
        case WM_MOUSELEAVE:
            if (hoverX != -1 || hoverY != -1) {
                hoverX = hoverY = -1;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
            
        case WM_LBUTTONDOWN:
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                int basePixelWidth = (rect.right - rect.left) / VGA_CHAR_WIDTH;
                int basePixelHeight = (rect.bottom - rect.top) / VGA_CHAR_HEIGHT;
                int pixelWidth = (basePixelWidth * g_zoomLevel) / 2;
                int pixelHeight = (basePixelHeight * g_zoomLevel) / 2;
                
                int x = LOWORD(lParam) / pixelWidth;
                int y = HIWORD(lParam) / pixelHeight;
                
                if (x >= 0 && x < VGA_CHAR_WIDTH && y >= 0 && y < VGA_CHAR_HEIGHT) {
                    BOOL oldValue = GetFontPixel(&g_font, g_selectedChar, x, y);
                    drawMode = TRUE;
                    if (oldValue != TRUE) {
                        // Add undo action for pixel change
                        AddUndoAction(ACTION_PIXEL_CHANGE, g_selectedChar, x, y, oldValue, TRUE);
                        SetFontPixel(&g_font, g_selectedChar, x, y, TRUE);
                        InvalidateRect(hwnd, NULL, FALSE);
                        InvalidateRect(g_hCharGrid, NULL, FALSE);
                    }
                }
                
                SetCapture(hwnd);
                isDragging = TRUE;
                
                // Mouse tracking
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);
            }
            break;
            
        case WM_RBUTTONDOWN:
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                int basePixelWidth = (rect.right - rect.left) / VGA_CHAR_WIDTH;
                int basePixelHeight = (rect.bottom - rect.top) / VGA_CHAR_HEIGHT;
                int pixelWidth = (basePixelWidth * g_zoomLevel) / 2;
                int pixelHeight = (basePixelHeight * g_zoomLevel) / 2;
                
                int x = LOWORD(lParam) / pixelWidth;
                int y = HIWORD(lParam) / pixelHeight;
                
                if (x >= 0 && x < VGA_CHAR_WIDTH && y >= 0 && y < VGA_CHAR_HEIGHT) {
                    BOOL oldValue = GetFontPixel(&g_font, g_selectedChar, x, y);
                    drawMode = FALSE;
                    if (oldValue != FALSE) {
                        // Add undo action for pixel change
                        AddUndoAction(ACTION_PIXEL_CHANGE, g_selectedChar, x, y, oldValue, FALSE);
                        SetFontPixel(&g_font, g_selectedChar, x, y, FALSE);
                        InvalidateRect(hwnd, NULL, FALSE);
                        InvalidateRect(g_hCharGrid, NULL, FALSE);
                    }
                }
                
                SetCapture(hwnd);
                isDragging = TRUE;
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
                    {
                        // Store old data for undo
                        unsigned char oldData[VGA_CHAR_HEIGHT];
                        unsigned char newData[VGA_CHAR_HEIGHT];
                        memcpy(oldData, g_font.data[g_selectedChar], VGA_CHAR_HEIGHT);
                        
                        // Clear the current character
                        ClearCharacter(&g_font, g_selectedChar);
                        memcpy(newData, g_font.data[g_selectedChar], VGA_CHAR_HEIGHT);
                        
                        // Add to undo system
                        AddUndoActionCharacter(ACTION_CHARACTER_CLEAR, g_selectedChar, oldData, newData);
                        
                        InvalidateRect(hwnd, NULL, FALSE);
                        InvalidateRect(g_hCharGrid, NULL, FALSE);
                    }
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
