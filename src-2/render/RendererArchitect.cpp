#include "Renderer.h"
#include "ArchitectConfig.h"
#include <cstdio>
#include <cmath>

void DrawArchitect(HDC hdc, const GameState& gs, const RenderResources& res) {
    const auto& arch = gs.architect;
    if (arch.state == GameState::HIDDEN) return;

    const int FONE = 65536;
    int SCALE = ArchitectConfig::BASE_SCALE; 
    const int DIST = ArchitectConfig::RENDER_DIST;
    const int CX_OFF = W / 2;
    const int TARGET_CY_OFF = ArchitectConfig::HUD_Y_OFFSET;
    const int CY_OFF = (int)(TARGET_CY_OFF * arch.yOffset);

    double pulseScale = 1.0;
    if (arch.malice > ArchitectConfig::MALICE_THRESHOLD) {
        int amp = arch.malice - ArchitectConfig::MALICE_THRESHOLD;
        pulseScale = 1.0 + (std::sin(arch.angX * PI / 180.0) * amp * ArchitectConfig::PULSE_AMPLITUDE);
    }
    int dynamicScale = (int)(SCALE * pulseScale);

    struct Point3D { int x, y, z; };
    Point3D verts[8] = {
        {-dynamicScale*FONE, -dynamicScale*FONE, -dynamicScale*FONE},
        { dynamicScale*FONE, -dynamicScale*FONE, -dynamicScale*FONE},
        { dynamicScale*FONE,  dynamicScale*FONE, -dynamicScale*FONE},
        {-dynamicScale*FONE,  dynamicScale*FONE, -dynamicScale*FONE},
        {-dynamicScale*FONE, -dynamicScale*FONE,  dynamicScale*FONE},
        { dynamicScale*FONE, -dynamicScale*FONE,  dynamicScale*FONE},
        { dynamicScale*FONE,  dynamicScale*FONE,  dynamicScale*FONE},
        {-dynamicScale*FONE,  dynamicScale*FONE,  dynamicScale*FONE}
    };

    static unsigned char edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    auto getSin = [](int angle) { return std::sin(angle * PI / 180.0); };
    auto getCos = [](int angle) { return std::cos(angle * PI / 180.0); };

    double sX = getSin(arch.angX);
    double cX = getCos(arch.angX);
    double sY = getSin(arch.angY);
    double cY = getCos(arch.angY);
    double sZ = getSin(arch.angZ);
    double cZ = getCos(arch.angZ);

    POINT proj[8];
    for (int i = 0; i < 8; i++) {
        double x = (double)verts[i].x / 65536.0;
        double y = (double)verts[i].y / 65536.0;
        double z = (double)verts[i].z / 65536.0;

        // Y rot
        double tx1 = x * cY + z * sY;
        double tz1 = z * cY - x * sY;
        x = tx1; z = tz1;

        // X rot
        double ty1 = y * cX - z * sX;
        double tz2 = z * cX + y * sX;
        y = ty1; z = tz2;

        // Z rot
        double tx2 = x * cZ - y * sZ;
        double ty2 = y * cZ + x * sZ;
        x = tx2; y = ty2;

        double dz = z + (double)DIST;
        if (dz < 1.0) dz = 1.0;

        proj[i].x = CX_OFF + (int)(x * (double)DIST / dz);
        proj[i].y = CY_OFF + (int)(y * (double)DIST / dz);
    }

    HPEN archPen = CreatePen(PS_SOLID, 2, RGB(arch.curr_r, arch.curr_g, arch.curr_b));
    HPEN oldPen = (HPEN)SelectObject(hdc, archPen);
    for (int i = 0; i < 12; i++) {
        MoveToEx(hdc, proj[edges[i][0]].x, proj[edges[i][0]].y, NULL);
        LineTo(hdc, proj[edges[i][1]].x, proj[edges[i][1]].y);
    }
    SelectObject(hdc, oldPen);
    DeleteObject(archPen);

    // Fade out text alpha if disappearing
    int alpha = (int)(255 * arch.yOffset);
    if (alpha < 0) alpha = 0; if (alpha > 255) alpha = 255;

    char dbg[64];
    sprintf(dbg, "E: %d | %d | %d", arch.anger, arch.aggress, arch.malice);
    SetTextColor(hdc, RGB(arch.curr_r, arch.curr_g, arch.curr_b));
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, CX_OFF - 30, CY_OFF + 50, dbg, (int)strlen(dbg));

    // Step 2: Render typing text
    if (!arch.currentText.empty()) {
        static int cursorTimer = 0;
        cursorTimer++;
        bool showCursor = (cursorTimer / 10) % 2 == 0;

        HFONT hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
        
        RECT boundRc = { W / 4, CY_OFF + 80, 3 * W / 4, H };
        int currX = boundRc.left;
        int currY = boundRc.top;
        
        static float baseHue = 0;
        float hueSpd = (float)(arch.curr_spd >> 8) * ArchitectConfig::RAINBOW_SPEED_MULT;
        baseHue = std::fmod(baseHue + hueSpd, 4.0f);
        int charCounter = 0;

        auto drawWord = [&](const std::string& w, bool highlight) {
            COLORREF wordOldCol = GetTextColor(hdc);
            SetBkMode(hdc, TRANSPARENT);
            SIZE fullWordSz;
            GetTextExtentPoint32A(hdc, w.c_str(), (int)w.length(), &fullWordSz);
            if (currX + fullWordSz.cx > boundRc.right && currX > boundRc.left) {
                currX = boundRc.left;
                currY += fullWordSz.cy;
            }

            if (highlight) {
                for (size_t i = 0; i < w.length(); ++i) {
                    float t = std::fmod(baseHue + charCounter * ArchitectConfig::RAINBOW_LETTER_OFFSET, 4.0f);
                    charCounter++;
                    float r=1, g=1, b=1;
                    if (t < 1.0f) { r = 1.0f - t; g = 1.0f; b = 0.0f; }
                    else if (t < 2.0f) { float lt = t - 1.0f; r = 0.0f; g = 1.0f - lt; b = lt; }
                    else if (t < 3.0f) { float lt = t - 2.0f; r = lt; g = lt; b = 1.0f; }
                    else { float lt = t - 3.0f; r = 1.0f; g = 1.0f; b = 1.0f - lt; }
                    SetTextColor(hdc, RGB((int)(r*255), (int)(g*255), (int)(b*255)));
                    char c = w[i];
                    SIZE sz;
                    GetTextExtentPoint32A(hdc, &c, 1, &sz);
                    TextOutA(hdc, currX, currY, &c, 1);
                    currX += sz.cx;
                }
            } else {
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, currX, currY, w.c_str(), (int)w.length());
                currX += fullWordSz.cx;
                charCounter += (int)w.length();
            }
            SetTextColor(hdc, wordOldCol);
        };

        std::string current = arch.currentText;
        size_t pos = 0;
        while (pos < current.length()) {
            size_t nextSpace = current.find_first_of(" .", pos);
            if (nextSpace == std::string::npos) nextSpace = current.length();
            else if (current[nextSpace] == '.' || current[nextSpace] == ' ') nextSpace++;
            std::string segment = current.substr(pos, nextSpace - pos);
            std::string clean = segment;
            while(!clean.empty() && (clean.back() == ' ' || clean.back() == '.')) clean.pop_back();
            bool highlight = false;
            for(int i=0; i < ArchitectConfig::HIGHLIGHT_COUNT; ++i) {
                if (clean == ArchitectConfig::HIGHLIGHT_WORDS[i]) { highlight = true; break; }
            }
            drawWord(segment, highlight);
            pos = nextSpace;
        }
        if (showCursor) TextOutA(hdc, currX, currY, "_", 1);
        SelectObject(hdc, oldFont);
        DeleteObject(hFont);
    }
}
