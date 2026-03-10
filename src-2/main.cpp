#include "Constants.h"
#include <stdint.h>
#include "ArchitectConfig.h"
#include "MathUtils.h"
#include "GameLogic.h"
#include "Renderer.h"

GameState g_gs = {
    { // maze
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,1,1,0,1,1,1,1,0,1,0,1,1,1,1,1,0,1},
        {1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,0,1,0,1,1,0,0,0,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,1,0,0,0,1,0,0,1,1,1,0,0,0,0,0,0,0,1},
        {1,1,1,0,1,0,1,1,1,0,1,0,1,0,0,1,0,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,0,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,0,0,0,1},
        {1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,0,1,0,1,0,1,1,1},
        {1,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1},
        {1,0,1,0,1,1,1,1,1,0,1,0,1,1,0,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,0,0,0,1,0,0,0,0,1,1,1,1,1,0,1,0,1},
        {1,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,1,0,1},
        {1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1},
        {1,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,1,1,1,1,0,1},
        {1,1,1,1,1,0,1,1,1,1,1,1,0,1,0,1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,1,1,1,1,1,1,1,1},
        {1,0,1,1,1,1,1,0,1,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,1,0,1,1,1,1,1,1,1,1,1,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    {0}, // visited
    false, // showFullMap
    1, // currentLevel
    10, // baseMazeSize
    1, 1, // logicalX, logicalY
    1,    // logicalDir
    1.5, 1.5, // camX, camY
    90.0,     // camAngle
    1.5, 1.5, // tgtX, tgtY
    90.0,     // tgtAngle
    LERP_NORMAL, // currentLerpSpeed
    false,       // isDashing
    {},          // entities
    0,           // enemiesKilled
    0,           // keysCollected
    0,           // totalKeys
    { GameState::HIDDEN, 0.0f, 0, 3, 3, 3, 0, 0x88, 0xFF, 256, 0, 0, 0, "", "", 2, 0, 0, 0, 0.0f } // Architect State (+ targetText, currentText, textCharDelay, textTimer, sentStart, sentEnd, curSpd)
};

RenderResources g_res;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, TIMER_ID, 1000 / FPS, NULL);
        srand(GetTickCount());
        ResetGame(g_gs);
        return 0;

    case WM_TIMER:
        g_gs.camX = lerp(g_gs.camX, g_gs.tgtX, g_gs.currentLerpSpeed);
        g_gs.camY = lerp(g_gs.camY, g_gs.tgtY, g_gs.currentLerpSpeed);
        if (abs(g_gs.camX - g_gs.tgtX) < 0.05 && abs(g_gs.camY - g_gs.tgtY) < 0.05) {
            g_gs.currentLerpSpeed = LERP_NORMAL;
            g_gs.isDashing = false;
        }
        {
            double diff = g_gs.tgtAngle - g_gs.camAngle;
            while (diff > 180.0) diff -= 360.0;
            while (diff < -180.0) diff += 360.0;
            g_gs.camAngle += diff * g_gs.currentLerpSpeed;
            g_gs.camAngle = angleWrap(g_gs.camAngle);
        }

        // Update Architect (Emotion Engine & State Machine)
        {
            auto& arch = g_gs.architect;

            if (arch.state != GameState::HIDDEN) {
                auto smooth = [](int& curr, int target, int divisor) {
                    if (curr == target) return;
                    int diff = target - curr;
                    int step = diff / divisor;
                    if (step == 0) step = (diff > 0) ? 1 : -1;
                    curr += step;
                };

                smooth(arch.curr_r, ArchitectConfig::COL_R[arch.anger], ArchitectConfig::COLOR_SMOOTH_DIVISOR);
                smooth(arch.curr_g, ArchitectConfig::COL_G[arch.anger], ArchitectConfig::COLOR_SMOOTH_DIVISOR);
                smooth(arch.curr_b, ArchitectConfig::COL_B[arch.anger], ArchitectConfig::COLOR_SMOOTH_DIVISOR);
                smooth(arch.curr_spd, ArchitectConfig::SPD_TGT[arch.aggress], ArchitectConfig::SPEED_SMOOTH_DIVISOR);

                int spd = arch.curr_spd >> 8;
                arch.angX = (arch.angX + spd) % 360;
                arch.angY = (arch.angY + (spd / 2 + 1)) % 360;
                arch.angZ = (arch.angZ + (spd / 4 + 1)) % 360;

                if (arch.angX < 0) arch.angX += 360;
                if (arch.angY < 0) arch.angY += 360;
                if (arch.angZ < 0) arch.angZ += 360;

                // State Machine Logic
                switch (arch.state) {
                case GameState::APPEARING:
                    arch.yOffset += 0.05f;
                    if (arch.yOffset >= 1.0f) {
                        arch.yOffset = 1.0f;
                        arch.state = GameState::TYPING;
                    }
                    break;

                case GameState::TYPING:
                    if (arch.currentText.length() < arch.targetText.length()) {
                        // Determine sentence boundaries if not set
                        if (arch.sentenceEndIdx <= (int)arch.currentText.length()) {
                            arch.sentenceStartIdx = (int)arch.currentText.length();
                            size_t nextDot = arch.targetText.find('.', arch.sentenceStartIdx);
                            if (nextDot == std::string::npos) arch.sentenceEndIdx = (int)arch.targetText.length();
                            else arch.sentenceEndIdx = (int)nextDot + 1;
                        }

                        int sentLen = arch.sentenceEndIdx - arch.sentenceStartIdx;
                        int typedInSent = (int)arch.currentText.length() - arch.sentenceStartIdx;
                        
                        if (sentLen > 0) {
                            float progress = (float)typedInSent / sentLen;
                            float speedFactor = 1.0f;
                            
                            if (progress <= 0.75f) {
                                float t = progress / 0.75f;
                                speedFactor = 1.0f + 3.0f * (t * t);
                            } else {
                                float t = (progress - 0.75f) / 0.25f;
                                speedFactor = 4.0f - 2.5f * t; 
                            }
                            
                            arch.currentSpeed = speedFactor * 0.5f; 
                        } else {
                            arch.currentSpeed = 1.0f;
                        }

                        static float accumulatedChars = 0;
                        accumulatedChars += arch.currentSpeed;
                        while (accumulatedChars >= 1.0f && arch.currentText.length() < arch.targetText.length()) {
                            arch.currentText += arch.targetText[arch.currentText.length()];
                            accumulatedChars -= 1.0f;
                            if ((int)arch.currentText.length() == arch.sentenceEndIdx) break;
                        }
                    } else {
                        arch.state = GameState::WAITING;
                    }
                    break;

                case GameState::WAITING:
                    if (arch.waitTimer > 0) {
                        arch.waitTimer--;
                    } else {
                        arch.state = GameState::DISAPPEARING;
                    }
                    break;

                case GameState::DISAPPEARING:
                    arch.yOffset -= 0.05f;
                    if (arch.yOffset <= 0.0f) {
                        arch.yOffset = 0.0f;
                        arch.state = GameState::HIDDEN;
                    }
                    break;
                }
            }
        }

        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_KEYDOWN:
        switch (wp) {
        case 'W': TryMove(g_gs, 1);  break;
        case 'S': TryMove(g_gs, -1); break;
        case VK_SPACE: TryMove(g_gs, 1, true); break;
        case 'A':
            g_gs.logicalDir = (g_gs.logicalDir + 3) % 4;
            g_gs.tgtAngle = g_gs.logicalDir * 90.0;
            RevealLineOfSight(g_gs);
            break;
        case 'D':
            g_gs.logicalDir = (g_gs.logicalDir + 1) % 4;
            g_gs.tgtAngle = g_gs.logicalDir * 90.0;
            RevealLineOfSight(g_gs);
            break;
        case 'Q':
            PostQuitMessage(0);
            break;
        case 'R':
            ResetGame(g_gs);
            break;
        case 'T':
            g_gs.showFullMap = !g_gs.showFullMap;
            break;
        case 'O': // Decrease emotions
            if (g_gs.architect.anger > ArchitectConfig::MIN_LEVEL) {
                g_gs.architect.anger--;
                g_gs.architect.aggress--;
                g_gs.architect.malice--;
            }
            break;
        case 'P': // Increase emotions
            if (g_gs.architect.anger < ArchitectConfig::MAX_LEVEL) {
                g_gs.architect.anger++;
                g_gs.architect.aggress++;
                g_gs.architect.malice++;
            }
            break;
        case 'I': // Step 3 Test: Trigger Notification
            TriggerArchitect(g_gs, 5, 5, 5, "I SEE YOU RUNNING THROUGH MY MAZE. THERE IS NO ESCAPE FROM THE ARCHITECT. THE MAZE IS INFINITE.", 3000);
            break;
        case VK_OEM_PLUS: // '+' key
        case VK_ADD:      // Numpad '+'
            g_gs.currentLevel++;
            ResetGame(g_gs);
            break;
        case VK_OEM_MINUS: // '-' key
        case VK_SUBTRACT:  // Numpad '-'
            if (g_gs.currentLevel > 1) {
                g_gs.currentLevel--;
                ResetGame(g_gs);
            }
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
        FillRect(memDC, &rc, g_res.blackBrush);
        Render3D(memDC, g_gs, g_res);
        DrawMinimap(memDC, g_gs, g_res);
        DrawArchitect(memDC, g_gs, g_res);
        DrawHUD(memDC, g_gs);
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
    InitResources(g_res);
    WNDCLASSEXA wc = {};
    wc.cbSize       = sizeof(wc);
    wc.lpfnWndProc  = WndProc;
    wc.hInstance    = hInst;
    wc.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName= "MazeV2Class";
    wc.hbrBackground= g_res.blackBrush;
    RegisterClassExA(&wc);
    RECT wr = { 0, 0, W, H };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExA(0, "MazeV2Class", "Wireframe Maze Engine (Phase 4) - Modular",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInst, NULL);
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    DeleteResources(g_res);
    return (int)msg.wParam;
}
