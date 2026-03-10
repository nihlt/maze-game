#include "Renderer.h"
#include <cstdio>
#include <cstring>
#include <cmath>

void DrawMinimap(HDC hdc, const GameState& gs, const RenderResources& res) {
    const int cellSz = 6;
    const int offX = W - MW * cellSz - 10;
    const int offY = 10;

    for (int y = 0; y < MH; y++) {
        for (int x = 0; x < MW; x++) {
            if (!gs.showFullMap && gs.visited[y][x] == 0) continue;
            RECT rc = { offX + x * cellSz, offY + y * cellSz, offX + (x + 1) * cellSz, offY + (y + 1) * cellSz };
            if (gs.maze[y][x] == 1) {
                HBRUSH wb = CreateSolidBrush(RGB(0, 80, 0));
                FillRect(hdc, &rc, wb);
                DeleteObject(wb);
            } else {
                HBRUSH fb = CreateSolidBrush(RGB(20, 20, 20));
                FillRect(hdc, &rc, fb);
                DeleteObject(fb);
            }
        }
    }
    int pcx = offX + (int)(gs.camX * cellSz);
    int pcy = offY + (int)(gs.camY * cellSz);
    SelectObject(hdc, res.bluePen);
    Ellipse(hdc, pcx - 2, pcy - 2, pcx + 3, pcy + 3);
    
    double rad = gs.camAngle * PI / 180.0;
    MoveToEx(hdc, pcx, pcy, NULL);
    LineTo(hdc, pcx + (int)(std::sin(rad) * 10), pcy - (int)(std::cos(rad) * 10));

    for (const auto& e : gs.entities) {
        if (!e.active || (!gs.showFullMap && gs.visited[e.y][e.x] == 0)) continue;
        int ecx = offX + e.x * cellSz + cellSz / 2;
        int ecy = offY + e.y * cellSz + cellSz / 2;
        if (e.type == ENTITY_KEY) {
            SelectObject(hdc, res.yellowPen);
            Ellipse(hdc, ecx - 1, ecy - 1, ecx + 2, ecy + 2);
            continue;
        }
        if (e.type == ENEMY_PATROL) SelectObject(hdc, res.bluePen);
        else if (e.type == ENEMY_GLITCH) {
            HPEN cyanPen = CreatePen(PS_SOLID, 1, RGB(0, 200, 255));
            SelectObject(hdc, cyanPen);
            Ellipse(hdc, ecx - 2, ecy - 2, ecx + 3, ecy + 3);
            DeleteObject(cyanPen);
            continue;
        }
        else SelectObject(hdc, res.redPen);
        Ellipse(hdc, ecx - 2, ecy - 2, ecx + 3, ecy + 3);
    }
}

void DrawHUD(HDC hdc, const GameState& gs) {
    SetTextColor(hdc, RGB(0, 255, 0));
    SetBkMode(hdc, TRANSPARENT);
    const char* dirNames[] = { "NORTH", "EAST", "SOUTH", "WEST" };
    char buf[256];
    int activeCount = 0;
    for(const auto& e : gs.entities) if(e.active) activeCount++;
    sprintf(buf, "Logical: [%d, %d] %s | Visual: [%.1f, %.1f] %.0f\nEnemies: %d Active | %d Killed | Keys: %d", 
            gs.logicalX, gs.logicalY, dirNames[gs.logicalDir], gs.camX, gs.camY, gs.camAngle, activeCount, gs.enemiesKilled, gs.keysCollected);
    RECT textRc = {10, 10, 800, 100};
    DrawTextA(hdc, buf, -1, &textRc, DT_LEFT | DT_TOP);
    const char* help = "WASD - Move/Turn  |  SPACE - Dash  |  Q - Quit";
    TextOutA(hdc, 10, H - 30, help, (int)strlen(help));
}
