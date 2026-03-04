#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

/* TODO: windowing/messages */
/* TODO: wgl + extensions + gl context */
/* TODO: break apart
int main(void) {
    HDC hScreenDC = GetDC(NULL);
    if (!hScreenDC) return -1;

    BITMAP structBitmapHeader;
    memset( &structBitmapHeader, 0, sizeof(BITMAP) );

    HGDIOBJ hGDIObject = GetCurrentObject(hScreenDC, OBJ_BITMAP);
    GetObject(hGDIObject, sizeof(BITMAP), &structBitmapHeader);

    int width = structBitmapHeader.bmWidth;
    int height = structBitmapHeader.bmHeight;

    HDC hMemDC = CreateCompatibleDC(hScreenDC);
    if (!hMemDC) { ReleaseDC(NULL, hScreenDC); return -1; }

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;    // request 32bpp (BGRA)
    bmi.bmiHeader.biCompression = BI_RGB;

    // Create DIB section so we can access pixels directly
    void *pBits = NULL;
    HBITMAP hBitmap = CreateDIBSection(hScreenDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    if (!hBitmap || !pBits) {
        DeleteDC(hMemDC);
        ReleaseDC(NULL, hScreenDC);
        return -1;
    }

    HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);
    if (!hOld) {
        DeleteObject(hBitmap);
        DeleteDC(hMemDC);
        ReleaseDC(NULL, hScreenDC);
        return -1;
    }

    // BitBlt from screen to our memory DC
    if (!BitBlt(hMemDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY | CAPTUREBLT)) {
        SelectObject(hMemDC, hOld);
        DeleteObject(hBitmap);
        DeleteDC(hMemDC);
        ReleaseDC(NULL, hScreenDC);
        return -1;
    }

    // Determine composition (bytes per pixel)
    int bytesPerPixel = 4; // because we requested 32bpp
    size_t image_length = (size_t)width * (size_t)height * bytesPerPixel;

    // Allocate return buffer and copy pixel data
    uint8_t *buf = (uint8_t *)malloc(image_length);
    if (!buf) {
        SelectObject(hMemDC, hOld);
        DeleteObject(hBitmap);
        DeleteDC(hMemDC);
        ReleaseDC(NULL, hScreenDC);
        return -1;
    }

    // pBits contains BGRA (B,G,R,A) in little-endian order per pixel
    memcpy(buf, pBits, image_length);

    // Cleanup
    SelectObject(hMemDC, hOld);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hScreenDC);

    return 0;
}
*/
#endif
