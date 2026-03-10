#include <windows.h>
#include <stdint.h>
#include <stdio.h>

// ---- constants ----
#define FP       16
#define FONE     65536
#define SCALE    100
#define DIST     400
#define WW       800
#define WH       600
#define TID      1
#define TMS      16
#define NV       8
#define NE       12
#define BLACKNESS 0x00000042

// ---- data ----
const char szCls[] = "Cube3D";
const char szTtl[] = "The Architect's Cube - C++ Interface";

// Color gradients for Anger levels 1..13
const uint8_t col_r[] = {0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0xFF, 0xFF, 0xFF, 0xFF, 0xCC, 0xAA, 0x88};
const uint8_t col_g[] = {0, 0x00, 0x88, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0x55, 0x00, 0x00, 0x00, 0x00};
const uint8_t col_b[] = {0, 0xFF, 0xFF, 0xFF, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0xAA, 0xFF};

// Target rotation speed * 256 for interpolation
const int32_t spd_tgt[] = {0, 256, 307, 384, 460, 563, 665, 793, 921, 1075, 1228, 1408, 1587, 1792};

// 8 vertices 
struct Point3D {
    int32_t x, y, z;
};

Point3D verts[NV] = {
    {-SCALE * FONE, -SCALE * FONE, -SCALE * FONE},
    { SCALE * FONE, -SCALE * FONE, -SCALE * FONE},
    { SCALE * FONE,  SCALE * FONE, -SCALE * FONE},
    {-SCALE * FONE,  SCALE * FONE, -SCALE * FONE},
    {-SCALE * FONE, -SCALE * FONE,  SCALE * FONE},
    { SCALE * FONE, -SCALE * FONE,  SCALE * FONE},
    { SCALE * FONE,  SCALE * FONE,  SCALE * FONE},
    {-SCALE * FONE,  SCALE * FONE,  SCALE * FONE}
};

// 12 edges
uint8_t edg[NE * 2] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

// sin LUT 360 entries
int32_t stab[360] = {
    0, 1143, 2287, 3429, 4571, 5711, 6850, 7986, 9120, 10252,
    11380, 12504, 13625, 14742, 15854, 16961, 18064, 19160, 20251, 21336,
    22414, 23486, 24550, 25606, 26655, 27696, 28729, 29752, 30767, 31772,
    32768, 33753, 34728, 35693, 36647, 37589, 38521, 39440, 40347, 41243,
    42125, 42995, 43852, 44695, 45525, 46340, 47142, 47929, 48702, 49460,
    50203, 50931, 51643, 52339, 53019, 53683, 54331, 54963, 55577, 56175,
    56755, 57319, 57864, 58393, 58903, 59395, 59870, 60326, 60763, 61183,
    61583, 61965, 62328, 62672, 62997, 63302, 63589, 63856, 64103, 64331,
    64540, 64729, 64898, 65047, 65176, 65286, 65376, 65446, 65496, 65526,
    65536, 65526, 65496, 65446, 65376, 65286, 65176, 65047, 64898, 64729,
    64540, 64331, 64103, 63856, 63589, 63302, 62997, 62672, 62328, 61965,
    61583, 61183, 60763, 60326, 59870, 59395, 58903, 58393, 57864, 57319,
    56755, 56175, 55577, 54963, 54331, 53683, 53019, 52339, 51643, 50931,
    50203, 49460, 48702, 47929, 47142, 46340, 45525, 44695, 43852, 42995,
    42125, 41243, 40347, 39440, 38521, 37589, 36647, 35693, 34728, 33753,
    32768, 31772, 30767, 29752, 28729, 27696, 26655, 25606, 24550, 23486,
    22414, 21336, 20251, 19160, 18064, 16961, 15854, 14742, 13625, 12504,
    11380, 10252, 9120, 7986, 6850, 5711, 4571, 3429, 2287, 1143,
    0, -1143, -2287, -3429, -4571, -5711, -6850, -7986, -9120, -10252,
    -11380, -12504, -13625, -14742, -15854, -16961, -18064, -19160, -20251, -21336,
    -22414, -23486, -24550, -25606, -26655, -27696, -28729, -29752, -30767, -31772,
    -32768, -33753, -34728, -35693, -36647, -37589, -38521, -39440, -40347, -41243,
    -42125, -42995, -43852, -44695, -45525, -46340, -47142, -47929, -48702, -49460,
    -50203, -50931, -51643, -52339, -53019, -53683, -54331, -54963, -55577, -56175,
    -56755, -57319, -57864, -58393, -58903, -59395, -59870, -60326, -60763, -61183,
    -61583, -61965, -62328, -62672, -62997, -63302, -63589, -63856, -64103, -64331,
    -64540, -64729, -64898, -65047, -65176, -65286, -65376, -65446, -65496, -65526,
    -65536, -65526, -65496, -65446, -65376, -65286, -65176, -65047, -64898, -64729,
    -64540, -64331, -64103, -63856, -63589, -63302, -62997, -62672, -62328, -61965,
    -61583, -61183, -60763, -60326, -59870, -59395, -58903, -58393, -57864, -57319,
    -56755, -56175, -55577, -54963, -54331, -53683, -53019, -52339, -51643, -50931,
    -50203, -49460, -48702, -47929, -47142, -46340, -45525, -44695, -43852, -42995,
    -42125, -41243, -40347, -39440, -38521, -37589, -36647, -35693, -34728, -33753,
    -32768, -31772, -30767, -29752, -28729, -27696, -26655, -25606, -24550, -23486,
    -22414, -21336, -20251, -19160, -18064, -16961, -15854, -14742, -13625, -12504,
    -11380, -10252, -9120, -7986, -6850, -5711, -4571, -3429, -2287, -1143
};

// ---- bss ----
struct Point2D {
    int32_t x, y;
};
Point2D proj[NV];
int32_t angX = 0, angY = 0, angZ = 0;

// Emotion System
int32_t anger = 3, aggress = 3, malice = 3;
int32_t curr_r = 0, curr_g = 0x88, curr_b = 0xFF;
int32_t curr_spd = 256;

char input_buf[1024] = {0};
int32_t input_len = 0;
int32_t api_state = 0; // 0=idle, 1=loading

HINSTANCE g_hinst;
HPEN g_pen = NULL;
RECT rc_buf;

// ---- functions ----

void MoveVal_Smooth(int32_t *addr, int32_t target, int32_t divisor) {
    int32_t curr = *addr;
    if (curr == target) return;
    int32_t diff = target - curr;
    int32_t step = diff / divisor;
    if (step == 0) {
        if (diff > 0) step = 1;
        else if (diff < 0) step = -1;
    }
    *addr += step;
}

void Transform() {
    int32_t tsinx = stab[angX];
    int32_t tcosx = stab[(angX + 90) % 360];
    int32_t tsiny = stab[angY];
    int32_t tcosy = stab[(angY + 90) % 360];
    int32_t tsinz = stab[angZ];
    int32_t tcosz = stab[(angZ + 90) % 360];

    int32_t tcx = (rc_buf.right - rc_buf.left) / 2;
    int32_t tcy = (rc_buf.bottom - rc_buf.top) / 2;

    for (int i = 0; i < NV; i++) {
        int32_t x = verts[i].x;
        int32_t y = verts[i].y;
        int32_t z = verts[i].z;

        // Scale Pulsation (Malice)
        if (malice > 8) {
            int32_t amp = malice - 8;
            int32_t scale_add = (tsinx * amp) >> 6;
            int32_t dynamic_scale = FONE + scale_add;
            x = (int32_t)(((int64_t)x * dynamic_scale) >> FP);
            y = (int32_t)(((int64_t)y * dynamic_scale) >> FP);
            z = (int32_t)(((int64_t)z * dynamic_scale) >> FP);
        }

        // Y rot
        int32_t x1 = (int32_t)(((int64_t)x * tcosy + (int64_t)z * tsiny) >> FP);
        int32_t z1 = (int32_t)(((int64_t)z * tcosy - (int64_t)x * tsiny) >> FP);
        x = x1; z = z1;

        // X rot
        int32_t y1 = (int32_t)(((int64_t)y * tcosx - (int64_t)z * tsinx) >> FP);
        int32_t z2 = (int32_t)(((int64_t)z * tcosx + (int64_t)y * tsinx) >> FP);
        y = y1; z = z2;

        // Z rot
        int32_t x2 = (int32_t)(((int64_t)x * tcosz - (int64_t)y * tsinz) >> FP);
        int32_t y2 = (int32_t)(((int64_t)y * tcosz + (int64_t)x * tsinz) >> FP);
        x = x2; y = y2;

        // perspective
        x >>= FP;
        y >>= FP;
        z >>= FP;

        int32_t depth = z + DIST;
        if (depth < 1) depth = 1;

        proj[i].x = (x * DIST) / depth + tcx;
        proj[i].y = (y * DIST) / depth + tcy;
    }
}

// Simulated AI process (until integrated with actual API)
DWORD WINAPI ThreadProc(LPVOID lpParam) {
    Sleep(2000);
    // Dummy parsing logic for demo
    if (strstr(input_buf, "anger")) anger = 10;
    if (strstr(input_buf, "max")) { anger = 13; aggress = 13; malice = 13; }
    if (strstr(input_buf, "min")) { anger = 1; aggress = 1; malice = 1; }
    
    api_state = 0;
    input_len = 0;
    input_buf[0] = 0;
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, TID, TMS, NULL);
            return 0;

        case WM_TIMER: {
            // Update colors based on anger
            int32_t tr = col_r[anger];
            int32_t tg = col_g[anger];
            int32_t tb = col_b[anger];
            MoveVal_Smooth(&curr_r, tr, 8);
            MoveVal_Smooth(&curr_g, tg, 8);
            MoveVal_Smooth(&curr_b, tb, 8);

            COLORREF col = RGB(curr_r & 0xFF, curr_g & 0xFF, curr_b & 0xFF);
            if (g_pen) DeleteObject(g_pen);
            g_pen = CreatePen(PS_SOLID, 2, col);

            // Update speed based on aggressiveness
            int32_t target_spd = spd_tgt[aggress];
            MoveVal_Smooth(&curr_spd, target_spd, 16);
            int32_t spd = curr_spd >> 8;

            angX = (angX + spd) % 360;
            angY = (angY + (spd / 2 + 1)) % 360;
            angZ = (angZ + (spd / 4 + 1)) % 360;

            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rc_buf);

            PatBlt(hdc, 0, 0, rc_buf.right, rc_buf.bottom, BLACKNESS);

            SelectObject(hdc, g_pen);
            Transform();

            for (int i = 0; i < NE; i++) {
                int v1 = edg[i * 2];
                int v2 = edg[i * 2 + 1];
                MoveToEx(hdc, proj[v1].x, proj[v1].y, NULL);
                LineTo(hdc, proj[v2].x, proj[v2].y);
            }

            // Text Overlays
            char dbg[128];
            sprintf(dbg, "Emotion Engine: Ang: %d | Agg: %d | Mal: %d", anger, aggress, malice);
            TextOutA(hdc, 10, 10, dbg, strlen(dbg));

            const char* status = (api_state == 0) ? "Idle. Waiting for your query..." : "Thinking... Communicating with the Singularity...";
            TextOutA(hdc, 10, 35, status, strlen(status));

            if (input_len > 0) {
                TextOutA(hdc, 10, 60, input_buf, input_len);
            }

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CHAR:
            if (wp == 8) { // Backspace
                if (input_len > 0) input_buf[--input_len] = 0;
            } else if (wp == 13) { // Enter
                if (api_state == 0 && input_len > 0) {
                    api_state = 1;
                    CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
                }
            } else if (wp >= 32) {
                if (input_len < 1022) {
                    input_buf[input_len++] = (char)wp;
                    input_buf[input_len] = 0;
                }
            }
            return 0;

        case WM_KEYDOWN:
            if (wp == VK_ESCAPE) DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, TID);
            if (g_pen) DeleteObject(g_pen);
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcA(hwnd, msg, wp, lp);
    }
}

int main() {
    g_hinst = GetModuleHandle(NULL);

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW | 0x0020;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = g_hinst;
    wc.lpszClassName = szCls;

    if (!RegisterClassExA(&wc)) return 1;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - WW) / 2;
    int posY = (screenH - WH) / 2;

    HWND hwnd = CreateWindowExA(0, szCls, szTtl, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                posX, posY, WW, WH, NULL, NULL, g_hinst, NULL);

    if (!hwnd) return 1;

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
