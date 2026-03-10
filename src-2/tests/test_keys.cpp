#include "Constants.h"
#include "MathUtils.h"
#include "GameLogic.h"
#include "Renderer.h"
#include <vector>
#include <cstdio>

// Mock GameState with key
GameState test_gs = {
    { // maze
        {1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,0,1},
        {1,0,1,0,0,0,0,1,0,1},
        {1,0,1,0,1,1,0,1,0,1},
        {1,0,1,0,1,0,0,1,0,1},
        {1,0,1,0,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1},
    },
    {0}, // visited
    1, 1, // logicalX, logicalY
    1,    // logicalDir (East)
    1.5, 1.5, // camX, camY
    90.0,     // camAngle
    1.5, 1.5, // tgtX, tgtY
    90.0,     // tgtAngle
    LERP_NORMAL, // currentLerpSpeed
    false,       // isDashing
    {},          // entities
    0,           // enemiesKilled
    0            // keysCollected
};

RenderResources test_res;

LRESULT CALLBACK TestWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, TIMER_ID, 1000 / FPS, NULL);
        test_gs.entities.clear();
        // Place a key right in front of the player (player is at 1,1 looking East, so key at 2,1)
        test_gs.entities.push_back({ENTITY_KEY, 2, 1, 0, true});
        // Place another key further away
        test_gs.entities.push_back({ENTITY_KEY, 5, 1, 0, true});
        RevealLineOfSight(test_gs);
        return 0;

    case WM_TIMER:
        test_gs.camX = lerp(test_gs.camX, test_gs.tgtX, test_gs.currentLerpSpeed);
        test_gs.camY = lerp(test_gs.camY, test_gs.tgtY, test_gs.currentLerpSpeed);
        {
            double diff = test_gs.tgtAngle - test_gs.camAngle;
            while (diff > 180.0) diff -= 360.0;
            while (diff < -180.0) diff += 360.0;
            test_gs.camAngle += diff * test_gs.currentLerpSpeed;
            test_gs.camAngle = angleWrap(test_gs.camAngle);
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_KEYDOWN:
        switch (wp) {
        case 'W': TryMove(test_gs, 1);  break;
        case 'S': TryMove(test_gs, -1); break;
        case 'A':
            test_gs.logicalDir = (test_gs.logicalDir + 3) % 4;
            test_gs.tgtAngle = test_gs.logicalDir * 90.0;
            RevealLineOfSight(test_gs);
            break;
        case 'D':
            test_gs.logicalDir = (test_gs.logicalDir + 1) % 4;
            test_gs.tgtAngle = test_gs.logicalDir * 90.0;
            RevealLineOfSight(test_gs);
            break;
        case 'R': // Reset for testing
            test_gs.logicalX = 1; test_gs.logicalY = 1;
            test_gs.tgtX = 1.5; test_gs.tgtY = 1.5;
            test_gs.keysCollected = 0;
            for(auto& e : test_gs.entities) e.active = true;
            break;
        case 'Q':
            PostQuitMessage(0);
            break;
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        SelectObject(memDC, bmp);
        FillRect(memDC, &rc, test_res.blackBrush);
        
        Render3D(memDC, test_gs, test_res);
        DrawMinimap(memDC, test_gs, test_res);
        DrawHUD(memDC, test_gs);
        
        char msgBuf[128];
        sprintf(msgBuf, "TEST MODE: Collect Yellow Keys! Keys: %d", test_gs.keysCollected);
        TextOutA(memDC, 10, H - 60, msgBuf, (int)strlen(msgBuf));
        TextOutA(memDC, 10, H - 45, "Press R to reset keys", 21);

        BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
        DeleteObject(bmp);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmd) {
    InitResources(test_res);
    WNDCLASSEXA wc = {};
    wc.cbSize       = sizeof(wc);
    wc.lpfnWndProc  = TestWndProc;
    wc.hInstance    = hInst;
    wc.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName= "KeyTestClass";
    wc.hbrBackground= test_res.blackBrush;
    RegisterClassExA(&wc);
    RECT wr = { 0, 0, W, H };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExA(0, "KeyTestClass", "Yellow Key Mechanic Test",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInst, NULL);
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    DeleteResources(test_res);
    return (int)msg.wParam;
}
