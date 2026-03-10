#include "Constants.h"
#include "MathUtils.h"
#include "GameLogic.h"
#include "Renderer.h"
#include "ArchitectConfig.h"
#include <vector>
#include <cstdio>

// Custom small test maze for portal testing
GameState portal_test_gs = {
    { // maze (10x10)
        {1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,1}, // Row 1: keys and portal will be here
        {1,1,1,1,1,1,1,1,0,1},
        {1,1,1,1,1,1,1,1,0,1},
        {1,1,1,1,1,1,1,1,0,1},
        {1,1,1,1,1,1,1,1,0,1},
        {1,1,1,1,1,1,1,1,0,1},
        {1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1},
    },
    {0}, // visited
    false, // showFullMap
    1, // currentLevel
    10, // baseMazeSize
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
    0,           // keysCollected
    2,           // totalKeys (Important!)
    { GameState::HIDDEN, 0.0f, 0, 3, 3, 3, 0, 0, 0, 256, 0, 0, 0, "", "", 2, 0, 0, 0, 0.0f } // Architect
};

RenderResources portal_test_res;

void SetupTestEntities(GameState& gs) {
    gs.entities.clear();
    // Place 2 keys
    gs.entities.push_back({ENTITY_KEY, 3, 1, 0, true});
    gs.entities.push_back({ENTITY_KEY, 5, 1, 0, true});
    // Place a portal at the end of the corridor
    gs.entities.push_back({ENTITY_PORTAL, 8, 1, 0, true});
    gs.keysCollected = 0;
    gs.totalKeys = 2;
    gs.logicalX = 1; gs.logicalY = 1;
    gs.tgtX = 1.5; gs.tgtY = 1.5;
    gs.camX = 1.5; gs.camY = 1.5;
    gs.logicalDir = 1; // East
    gs.camAngle = 90.0; gs.tgtAngle = 90.0;
    
    // Clear visited
    for(int y=0; y<MH; y++) for(int x=0; x<MW; x++) gs.visited[y][x] = 0;
    RevealLineOfSight(gs);
}

LRESULT CALLBACK PortalTestWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, TIMER_ID, 1000 / FPS, NULL);
        SetupTestEntities(portal_test_gs);
        return 0;

    case WM_TIMER:
        portal_test_gs.camX = lerp(portal_test_gs.camX, portal_test_gs.tgtX, portal_test_gs.currentLerpSpeed);
        portal_test_gs.camY = lerp(portal_test_gs.camY, portal_test_gs.tgtY, portal_test_gs.currentLerpSpeed);
        {
            double diff = portal_test_gs.tgtAngle - portal_test_gs.camAngle;
            while (diff > 180.0) diff -= 360.0;
            while (diff < -180.0) diff += 360.0;
            portal_test_gs.camAngle += diff * portal_test_gs.currentLerpSpeed;
            portal_test_gs.camAngle = angleWrap(portal_test_gs.camAngle);
        }
        
        // Architect State Machine (minimal update for typing effect)
        if (portal_test_gs.architect.state == GameState::TYPING) {
            if (portal_test_gs.architect.currentText.length() < portal_test_gs.architect.targetText.length()) {
                portal_test_gs.architect.currentText += portal_test_gs.architect.targetText[portal_test_gs.architect.currentText.length()];
            } else {
                portal_test_gs.architect.state = GameState::WAITING;
                portal_test_gs.architect.waitTimer = 100;
            }
        } else if (portal_test_gs.architect.state == GameState::WAITING) {
            if (portal_test_gs.architect.waitTimer > 0) portal_test_gs.architect.waitTimer--;
            else portal_test_gs.architect.state = GameState::HIDDEN;
        } else if (portal_test_gs.architect.state == GameState::APPEARING) {
             portal_test_gs.architect.state = GameState::TYPING;
        }

        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_KEYDOWN:
        switch (wp) {
        case 'W': TryMove(portal_test_gs, 1);  break;
        case 'S': TryMove(portal_test_gs, -1); break;
        case 'A':
            portal_test_gs.logicalDir = (portal_test_gs.logicalDir + 3) % 4;
            portal_test_gs.tgtAngle = portal_test_gs.logicalDir * 90.0;
            RevealLineOfSight(portal_test_gs);
            break;
        case 'D':
            portal_test_gs.logicalDir = (portal_test_gs.logicalDir + 1) % 4;
            portal_test_gs.tgtAngle = portal_test_gs.logicalDir * 90.0;
            RevealLineOfSight(portal_test_gs);
            break;
        case 'R': // Reset for testing
            portal_test_gs.currentLevel = 1;
            // Restore test maze in case it was overwritten by ResetGame (from portal transition)
            {
                int testMaze[10][10] = {
                    {1,1,1,1,1,1,1,1,1,1},
                    {1,0,0,0,0,0,0,0,0,1},
                    {1,1,1,1,1,1,1,1,0,1},
                    {1,1,1,1,1,1,1,1,0,1},
                    {1,1,1,1,1,1,1,1,0,1},
                    {1,1,1,1,1,1,1,1,0,1},
                    {1,1,1,1,1,1,1,1,0,1},
                    {1,1,1,1,1,1,1,1,0,1},
                    {1,0,0,0,0,0,0,0,0,1},
                    {1,1,1,1,1,1,1,1,1,1},
                };
                for(int y=0; y<10; y++) for(int x=0; x<10; x++) portal_test_gs.maze[y][x] = testMaze[y][x];
                for(int y=0; y<MH; y++) for(int x=0; x<MW; x++) if(y>=10 || x>=10) portal_test_gs.maze[y][x] = 1;
            }
            SetupTestEntities(portal_test_gs);
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
        FillRect(memDC, &rc, portal_test_res.blackBrush);
        
        Render3D(memDC, portal_test_gs, portal_test_res);
        DrawMinimap(memDC, portal_test_gs, portal_test_res);
        DrawHUD(memDC, portal_test_gs);
        DrawArchitect(memDC, portal_test_gs, portal_test_res);
        
        char msgBuf[128];
        SetTextColor(memDC, RGB(255, 255, 255));
        SetBkMode(memDC, TRANSPARENT);
        
        sprintf(msgBuf, "PORTAL TEST: Level %d", portal_test_gs.currentLevel);
        TextOutA(memDC, 10, H - 80, msgBuf, (int)strlen(msgBuf));
        
        sprintf(msgBuf, "Keys Collected: %d / %d", portal_test_gs.keysCollected, portal_test_gs.totalKeys);
        TextOutA(memDC, 10, H - 65, msgBuf, (int)strlen(msgBuf));
        
        const char* portalStatus = (portal_test_gs.keysCollected >= portal_test_gs.totalKeys) ? "OPEN" : "LOCKED";
        sprintf(msgBuf, "Portal Status: %s", portalStatus);
        TextOutA(memDC, 10, H - 50, msgBuf, (int)strlen(msgBuf));
        
        TextOutA(memDC, 10, H - 35, "Collect keys, then enter Portal. Press R to reset.", 51);

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
    InitResources(portal_test_res);
    WNDCLASSEXA wc = {};
    wc.cbSize       = sizeof(wc);
    wc.lpfnWndProc  = PortalTestWndProc;
    wc.hInstance    = hInst;
    wc.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName= "PortalTestClass";
    wc.hbrBackground= portal_test_res.blackBrush;
    RegisterClassExA(&wc);
    RECT wr = { 0, 0, W, H };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExA(0, "PortalTestClass", "Portal Mechanic Test",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInst, NULL);
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    DeleteResources(portal_test_res);
    return (int)msg.wParam;
}
