#include "Renderer.h"
#include "MathUtils.h"
#include "ArchitectConfig.h"
#include <vector>
#include <algorithm>
#include <cmath>

// ── 3D Face Representation ─────────────────────────────────────────────────
struct WorldFace {
    Vec2 p1; 
    Vec2 p2; 
    double colorIntensity; 
};

struct ScreenPoly {
    POINT pts[16];
    int numPts;
    double depth;
    int colorDepth; 
    
    bool operator<(const ScreenPoly& other) const {
        return depth > other.depth;
    }
};

std::vector<WorldFace> BuildWorldFaces(const GameState& gs) {
    std::vector<WorldFace> faces;
    const int render_rad = 8;
    int minX = std::max(0, (int)gs.camX - render_rad);
    int maxX = std::min(MW - 1, (int)gs.camX + render_rad);
    int minY = std::max(0, (int)gs.camY - render_rad);
    int maxY = std::min(MH - 1, (int)gs.camY + render_rad);

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            if (gs.maze[y][x] == 1) {
                double left = x, right = x + 1, top = y, bottom = y + 1;
                if (y == 0 || gs.maze[y-1][x] == 0)
                    faces.push_back({Vec2(left, top), Vec2(right, top), 1.0});
                if (y == MH-1 || gs.maze[y+1][x] == 0)
                    faces.push_back({Vec2(right, bottom), Vec2(left, bottom), 1.0});
                if (x == 0 || gs.maze[y][x-1] == 0)
                    faces.push_back({Vec2(left, bottom), Vec2(left, top), 1.0});
                if (x == MW-1 || gs.maze[y][x+1] == 0)
                    faces.push_back({Vec2(right, top), Vec2(right, bottom), 1.0});
            }
        }
    }
    return faces;
}

void Render3D(HDC hdc, const GameState& gs, const RenderResources& res) {
    std::vector<WorldFace> wFaces = BuildWorldFaces(gs);
    std::vector<ScreenPoly> sPolys;

    double rad = gs.camAngle * PI / 180.0;
    double fwdX = std::sin(rad), fwdY = -std::cos(rad);
    double rgtX = std::cos(rad), rgtY = std::sin(rad);

    for (const auto& wf : wFaces) {
        double fwX = wf.p2.x - wf.p1.x;
        double fwY = wf.p2.y - wf.p1.y;
        double nx = fwY;
        double ny = -fwX;
        double vx = wf.p1.x - gs.camX;
        double vy = wf.p1.y - gs.camY;

        if ((nx * vx + ny * vy) >= 0) continue;

        double dx1 = wf.p1.x - gs.camX, dy1 = wf.p1.y - gs.camY;
        double dx2 = wf.p2.x - gs.camX, dy2 = wf.p2.y - gs.camY;

        double lz1 = dx1 * fwdX + dy1 * fwdY;
        double lx1 = dx1 * rgtX + dy1 * rgtY;
        double lz2 = dx2 * fwdX + dy2 * fwdY;
        double lx2 = dx2 * rgtX + dy2 * rgtY;

        if (lz1 < Z_NEAR && lz2 < Z_NEAR) continue;

        if (lz1 < Z_NEAR) {
            double t = (Z_NEAR - lz1) / (lz2 - lz1);
            lx1 = lx1 + (lx2 - lx1) * t;
            lz1 = Z_NEAR;
        } else if (lz2 < Z_NEAR) {
            double t = (Z_NEAR - lz2) / (lz1 - lz2);
            lx2 = lx2 + (lx1 - lx2) * t;
            lz2 = Z_NEAR;
        }

        double centroidZ = (lz1 + lz2) / 2.0;
        int sx1 = CX + (int)(lx1 * FOV_SCALE / lz1);
        int sx2 = CX + (int)(lx2 * FOV_SCALE / lz2);
        int syTop1 = CY - (int)(WALL_HEIGHT * FOV_SCALE / lz1);
        int syBot1 = CY + (int)(WALL_HEIGHT * FOV_SCALE / lz1);
        int syTop2 = CY - (int)(WALL_HEIGHT * FOV_SCALE / lz2);
        int syBot2 = CY + (int)(WALL_HEIGHT * FOV_SCALE / lz2);

        ScreenPoly poly;
        poly.pts[0] = { sx1, syTop1 };
        poly.pts[1] = { sx2, syTop2 };
        poly.pts[2] = { sx2, syBot2 };
        poly.pts[3] = { sx1, syBot1 };
        poly.depth = centroidZ;
        poly.numPts = 4;
        
        if (centroidZ <= 2.5) poly.colorDepth = 0;
        else if (centroidZ <= 5.5) poly.colorDepth = 1;
        else poly.colorDepth = 2;

        sPolys.push_back(poly);
    }

    for (const auto& e : gs.entities) {
        if (!e.active) continue;
        double vx = e.x + 0.5 - gs.camX;
        double vy = e.y + 0.5 - gs.camY;
        double lx = vx * rgtX + vy * rgtY;
        double lz = vx * fwdX + vy * fwdY;
        if (lz < Z_NEAR || lz > 8.0) continue;
        
        int sx = CX + (int)(lx * FOV_SCALE / lz);
        int halfH = (int)(0.4 * FOV_SCALE / lz);
        int halfW = (int)(0.3 * FOV_SCALE / lz);
        int sy = CY + (int)(0.1 * FOV_SCALE / lz);

        // Prevent gigantic sizes when lz is very small
        if (halfW > W * 2) halfW = W * 2;
        if (halfH > H * 2) halfH = H * 2;
        
        ScreenPoly poly;
        poly.numPts = 3;
        poly.pts[0] = { sx, sy - halfH };
        poly.pts[1] = { sx + halfW, sy + halfH };
        poly.pts[2] = { sx - halfW, sy + halfH };

        if (e.type == ENEMY_STALKER) poly.colorDepth = 3;
        else if (e.type == ENEMY_PATROL) poly.colorDepth = 4;
        else if (e.type == ENEMY_GLITCH) poly.colorDepth = 5;
        else if (e.type == ENTITY_KEY) {
            poly.colorDepth = 6;
            poly.numPts = 4;
            poly.pts[0] = { sx, sy - halfH };
            poly.pts[1] = { sx + halfW, sy };
            poly.pts[2] = { sx, sy + halfH };
            poly.pts[3] = { sx - halfW, sy };
        } else if (e.type == ENTITY_PORTAL) {
            poly.colorDepth = 7;
            poly.numPts = 16; // Approximate a circle
            for (int i = 0; i < 16; i++) {
                double ang = i * 2.0 * PI / 16.0;
                int px = sx + (int)(std::cos(ang) * halfW * 1.5);
                int py = sy + (int)(std::sin(ang) * halfH * 1.5);
                poly.pts[i] = { px, py };
            }
        }
        poly.depth = lz;
        sPolys.push_back(poly);
    }

    std::sort(sPolys.begin(), sPolys.end());

    SelectObject(hdc, res.blackBrush);
    for (const auto& poly : sPolys) {
        if (poly.colorDepth == 0) SelectObject(hdc, res.greenPen);
        else if (poly.colorDepth == 1) SelectObject(hdc, res.dimPen);
        else if (poly.colorDepth == 2) SelectObject(hdc, res.darkPen);
        else if (poly.colorDepth == 3) SelectObject(hdc, res.redPen);
        else if (poly.colorDepth == 4) SelectObject(hdc, res.bluePen);
        else if (poly.colorDepth == 6) SelectObject(hdc, res.yellowPen);
        else if (poly.colorDepth == 7) {
            HPEN pPen = CreatePen(PS_SOLID, 2, RGB(ArchitectConfig::PORTAL_R, ArchitectConfig::PORTAL_G, ArchitectConfig::PORTAL_B));
            HGDIOBJ oldPen = SelectObject(hdc, pPen);
            if (gs.keysCollected >= gs.totalKeys) {
                HBRUSH hBrush = CreateHatchBrush(HS_DIAGCROSS, RGB(ArchitectConfig::PORTAL_R, ArchitectConfig::PORTAL_G, ArchitectConfig::PORTAL_B));
                HGDIOBJ oldBrush = SelectObject(hdc, hBrush);
                Polygon(hdc, poly.pts, poly.numPts);
                SelectObject(hdc, oldBrush);
                DeleteObject(hBrush);
            } else {
                HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
                Polygon(hdc, poly.pts, poly.numPts);
                SelectObject(hdc, oldBrush);
            }
            SelectObject(hdc, oldPen);
            DeleteObject(pPen);
            continue;
        }
        else if (poly.colorDepth == 5) {
            static int pulse = 0; pulse = (pulse + 10) % 255;
            HPEN pPen = CreatePen(PS_SOLID, 2, RGB(0, pulse, 200));
            SelectObject(hdc, pPen);
            Polygon(hdc, poly.pts, poly.numPts);
            DeleteObject(pPen);
            continue;
        }
        Polygon(hdc, poly.pts, poly.numPts);
    }
}
