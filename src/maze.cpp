// ============================================================================
// Wireframe Maze PoC - Advanced Architecture (v2)
// True 3D Math: World -> Camera -> Screen Space
// Features: Backface Culling, Frustum Clipping (Near Plane), Painter's Algorithm,
//           and Opaque Polygons (Black Brush + Green Pen) for perfect occlusion.
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>

// ── Window & Engine ────────────────────────────────────────────────────────
const int W = 800;
const int H = 600;
const int CX = W / 2;
const int CY = H / 2;
const int TIMER_ID = 1;
const int FPS = 60;
const double PI = 3.14159265358979323846;

// ── Map (24x24) ─────────────────────────────────────────────────────────────
const int MW = 24, MH = 24;
int maze[MH][MW] = {
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
};

int visited[MH][MW] = {0}; // 0 = fog, 1 = visited

// ── Player State (Logical & Visual) ────────────────────────────────────────
int logicalX = 1, logicalY = 1;
int logicalDir = 1; // 0=N, 1=E, 2=S, 3=W

// Camera Visual State (Interpolated)
double camX = 1.5, camY = 1.5; // World coords (center of cell)
double camAngle = 90.0;        // Degrees
double tgtX = 1.5, tgtY = 1.5;
double tgtAngle = 90.0;
double currentLerpSpeed = 0.15;
const double LERP_NORMAL = 0.15;
const double LERP_DASH = 0.40;  // Faster visual transition for dash
bool isDashing = false;         // Invincibility flag (for later when enemies are added)

// Camera configuration
const double FOV_SCALE = 600.0;     // focal length / zoom
const double WALL_HEIGHT = 0.6;     // height from eye level
const double Z_NEAR = 0.1;          // Near clipping plane

// ── GDI resources ──────────────────────────────────────────────────────────
HPEN greenPen, dimPen, darkPen, redPen, bluePen;
HBRUSH blackBrush;

// ── Entities ───────────────────────────────────────────────────────────────
enum EntityType { ENEMY_STALKER, ENEMY_PATROL, ENEMY_GLITCH };
struct Entity {
    EntityType type;
    int x, y;
    int dir; // For patrols
    bool active;
};
std::vector<Entity> entities;
int enemiesKilled = 0;

// ── Math Structures & Functions ─────────────────────────────────────────────
inline double lerp(double a, double b, double t) { return a + (b - a) * t; }
struct Vec2 {
    double x, y;
    Vec2() : x(0), y(0) {}
    Vec2(double _x, double _y) : x(_x), y(_y) {}
};

inline double angleWrap(double a) {
    while (a >= 360.0) a -= 360.0;
    while (a <  0.0)   a += 360.0;
    return a;
}

// ── 3D Face Representation ─────────────────────────────────────────────────
// Represents a single 2D wall segment in world space
struct WorldFace {
    Vec2 p1; // Left point (looking at wall face)
    Vec2 p2; // Right point
    double colorIntensity; // For fog/depth styling
};

// A face after transformation and clipping into Camera Space
struct CamFace {
    Vec2 p1; // Local X, Z
    Vec2 p2; // Local X, Z
    double depth; // Sorting key
};

// Represents a 2D projected polygon ready for GDI
struct ScreenPoly {
    POINT pts[4];
    int numPts;
    double depth;
    int colorDepth; // 0=Bright, 1=Medium, 2=Dark, 3=Red, 4=Blue
    
    // Sort descending (painter's algo)
    bool operator<(const ScreenPoly& other) const {
        return depth > other.depth; // Far elements drawn first
    }
};

// ── Logic: Generate World Faces ────────────────────────────────────────────
// Creates Line segments for walls in a radius around the player
std::vector<WorldFace> buildWorldFaces() {
    std::vector<WorldFace> faces;
    // Map bounds to limits around player to avoid rendering whole map
    const int render_rad = 8;
    int minX = std::max(0, (int)camX - render_rad);
    int maxX = std::min(MW - 1, (int)camX + render_rad);
    int minY = std::max(0, (int)camY - render_rad);
    int maxY = std::min(MH - 1, (int)camY + render_rad);

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            if (maze[y][x] == 1) {
                // A wall cell at (x,y) holds [x, x+1] and [y, y+1] in continuous space.
                double left = x, right = x + 1, top = y, bottom = y + 1;
                
                // Add faces if adjacent cell is empty (or out of bounds)
                // Note: Points are ordered Left->Right from the outside perspective 
                // to maintain consistent outward-facing normals.
                
                // NORTH face (Top edge)
                if (y == 0 || maze[y-1][x] == 0)
                    faces.push_back({Vec2(left, top), Vec2(right, top), 1.0});
                
                // SOUTH face (Bottom edge)
                if (y == MH-1 || maze[y+1][x] == 0)
                    faces.push_back({Vec2(right, bottom), Vec2(left, bottom), 1.0});
                    
                // WEST face (Left edge)
                if (x == 0 || maze[y][x-1] == 0)
                    faces.push_back({Vec2(left, bottom), Vec2(left, top), 1.0});
                    
                // EAST face (Right edge)
                if (x == MW-1 || maze[y][x+1] == 0)
                    faces.push_back({Vec2(right, top), Vec2(right, bottom), 1.0});
            }
        }
    }
    return faces;
}

// ── Rendering Pipeline ─────────────────────────────────────────────────────
void render3D(HDC hdc) {
    // 1. Get world faces
    std::vector<WorldFace> wFaces = buildWorldFaces();
    std::vector<ScreenPoly> sPolys;

    // Camera rotation vectors
    double rad = camAngle * PI / 180.0;
    // We treat 0=N(-y), 90=E(+x), 180=S(+y), 270=W(-x)
    double fwdX = sin(rad), fwdY = -cos(rad);
    double rgtX = cos(rad), rgtY = sin(rad);

    for (const auto& wf : wFaces) {
        // --- Backface Culling ---
        // Vector along the face (P1 to P2)
        double fwX = wf.p2.x - wf.p1.x;
        double fwY = wf.p2.y - wf.p1.y;
        
        // Outward Normal (rotate 90 deg clockwise) -> (fwY, -fwX)
        double nx = fwY;
        double ny = -fwX;

        // View vector from player to face's P1
        double vx = wf.p1.x - camX;
        double vy = wf.p1.y - camY;

        // Dot product: if > 0, normal points away from camera -> backface
        if ((nx * vx + ny * vy) >= 0) continue;

        // --- World to Camera Transformation ---
        // dx, dy
        double dx1 = wf.p1.x - camX, dy1 = wf.p1.y - camY;
        double dx2 = wf.p2.x - camX, dy2 = wf.p2.y - camY;

        // Transform to local Camera Space (LocalX points right, LocalZ points forward)
        // LocalZ = projection onto Forward vector
        // LocalX = projection onto Right vector
        double lz1 = dx1 * fwdX + dy1 * fwdY;
        double lx1 = dx1 * rgtX + dy1 * rgtY;
        
        double lz2 = dx2 * fwdX + dy2 * fwdY;
        double lx2 = dx2 * rgtX + dy2 * rgtY;

        // --- Near Plane Clipping ---
        if (lz1 < Z_NEAR && lz2 < Z_NEAR) continue; // Entirely behind us

        if (lz1 < Z_NEAR) {
            // Intersect line with Z = Z_NEAR
            double t = (Z_NEAR - lz1) / (lz2 - lz1);
            lx1 = lx1 + (lx2 - lx1) * t;
            lz1 = Z_NEAR;
        } else if (lz2 < Z_NEAR) {
            double t = (Z_NEAR - lz2) / (lz1 - lz2);
            lx2 = lx2 + (lx1 - lx2) * t;
            lz2 = Z_NEAR;
        }

        double centroidZ = (lz1 + lz2) / 2.0;
        
        // --- Perspective Projection ---
        // Screen X
        int sx1 = CX + (int)(lx1 * FOV_SCALE / lz1);
        int sx2 = CX + (int)(lx2 * FOV_SCALE / lz2);

        // Screen Y (Top / Bottom)
        int syTop1 = CY - (int)(WALL_HEIGHT * FOV_SCALE / lz1);
        int syBot1 = CY + (int)(WALL_HEIGHT * FOV_SCALE / lz1);
        int syTop2 = CY - (int)(WALL_HEIGHT * FOV_SCALE / lz2);
        int syBot2 = CY + (int)(WALL_HEIGHT * FOV_SCALE / lz2);

        // Prepare Polygon (4 vertices: TopLeft -> TopRight -> BottomRight -> BottomLeft)
        ScreenPoly poly;
        poly.pts[0] = { sx1, syTop1 };
        poly.pts[1] = { sx2, syTop2 };
        poly.pts[2] = { sx2, syBot2 };
        poly.pts[3] = { sx1, syBot1 };
        poly.depth = centroidZ;
        poly.numPts = 4;
        
        // Intensity/Depth color logic
        if (centroidZ <= 2.5) poly.colorDepth = 0;
        else if (centroidZ <= 5.5) poly.colorDepth = 1;
        else poly.colorDepth = 2;

        sPolys.push_back(poly);
    }

    // --- Entities (Billboards) ---
    for (const auto& e : entities) {
        if (!e.active) continue;
        
        double vx = e.x + 0.5 - camX;
        double vy = e.y + 0.5 - camY;
        
        double lx = vx * rgtX + vy * rgtY;
        double lz = vx * fwdX + vy * fwdY;
        
        if (lz < Z_NEAR || lz > 8.0) continue; // Fix occlusion: distance cap matching wall rad
        
        int sx = CX + (int)(lx * FOV_SCALE / lz);
        int halfH = (int)(0.4 * FOV_SCALE / lz);
        int halfW = (int)(0.3 * FOV_SCALE / lz);
        int sy = CY + (int)(0.1 * FOV_SCALE / lz);
        
        ScreenPoly poly;
        poly.numPts = 3;
        poly.pts[0] = { sx, sy - halfH };
        poly.pts[1] = { sx + halfW, sy + halfH };
        poly.pts[2] = { sx - halfW, sy + halfH };

        if (e.type == ENEMY_STALKER) {
            poly.colorDepth = 3; // Red
        } else if (e.type == ENEMY_PATROL) {
            poly.colorDepth = 4; // Blue
        } else if (e.type == ENEMY_GLITCH) {
            poly.colorDepth = 5; // Teal/Cyan for glitch
        }
        poly.depth = lz;
        
        sPolys.push_back(poly);
    }

    // 2. Painter's Algorithm: Sort Back-to-Front
    std::sort(sPolys.begin(), sPolys.end());

    // 3. Draw Polygons
    SelectObject(hdc, blackBrush);
    for (const auto& poly : sPolys) {
        if (poly.colorDepth == 0) SelectObject(hdc, greenPen);
        else if (poly.colorDepth == 1) SelectObject(hdc, dimPen);
        else if (poly.colorDepth == 2) SelectObject(hdc, darkPen);
        else if (poly.colorDepth == 3) SelectObject(hdc, redPen);
        else if (poly.colorDepth == 4) SelectObject(hdc, bluePen);
        else if (poly.colorDepth == 5) {
            // Pulse logic for glitch
            static int pulse = 0; pulse = (pulse + 10) % 255;
            HPEN pPen = CreatePen(PS_SOLID, 2, RGB(0, pulse, 200));
            SelectObject(hdc, pPen);
            Polygon(hdc, poly.pts, poly.numPts);
            DeleteObject(pPen);
            continue;
        }

        // Polygon draws both the edge (Pen) and the fill (Brush, which is Black)
        // This naturally occludes anything drawn behind it!
        Polygon(hdc, poly.pts, poly.numPts);
    }
}

// ── Minimap (with Fog of War) ──────────────────────────────────────────────
void drawMinimap(HDC hdc) {
    const int cellSz = 6;
    const int offX = W - MW * cellSz - 10;
    const int offY = 10;

    for (int y = 0; y < MH; y++) {
        for (int x = 0; x < MW; x++) {
            if (visited[y][x] == 0) continue; // Fog of War
            
            RECT rc = { offX + x * cellSz, offY + y * cellSz,
                        offX + (x + 1) * cellSz, offY + (y + 1) * cellSz };
            if (maze[y][x] == 1) {
                HBRUSH wb = CreateSolidBrush(RGB(0, 80, 0));
                FillRect(hdc, &rc, wb);
                DeleteObject(wb);
            } else {
                HBRUSH fb = CreateSolidBrush(RGB(20, 20, 20)); // Subtle floor indicator
                FillRect(hdc, &rc, fb);
                DeleteObject(fb);
            }
        }
    }
    // Player
    int pcx = offX + (int)(camX * cellSz);
    int pcy = offY + (int)(camY * cellSz);
    SelectObject(hdc, bluePen);
    Ellipse(hdc, pcx - 2, pcy - 2, pcx + 3, pcy + 3);
    
    // View ray
    double rad = camAngle * PI / 180.0;
    MoveToEx(hdc, pcx, pcy, NULL);
    LineTo(hdc, pcx + (int)(sin(rad) * 10), pcy - (int)(cos(rad) * 10));

    // Entities
    for (const auto& e : entities) {
        if (!e.active || visited[e.y][e.x] == 0) continue;
        int ecx = offX + e.x * cellSz + cellSz / 2;
        int ecy = offY + e.y * cellSz + cellSz / 2;
        if (e.type == ENEMY_PATROL) SelectObject(hdc, bluePen);
        else if (e.type == ENEMY_GLITCH) {
            HPEN cyanPen = CreatePen(PS_SOLID, 1, RGB(0, 200, 255));
            SelectObject(hdc, cyanPen);
            Ellipse(hdc, ecx - 2, ecy - 2, ecx + 3, ecy + 3);
            DeleteObject(cyanPen);
            continue;
        }
        else SelectObject(hdc, redPen);
        Ellipse(hdc, ecx - 2, ecy - 2, ecx + 3, ecy + 3);
    }
}

// ── HUD ────────────────────────────────────────────────────────────────────
void drawHUD(HDC hdc) {
    SetTextColor(hdc, RGB(0, 255, 0));
    SetBkMode(hdc, TRANSPARENT);
    const char* dirNames[] = { "NORTH", "EAST", "SOUTH", "WEST" };
    char buf[256];
    
    int activeCount = 0;
    for(const auto& e : entities) if(e.active) activeCount++;
    
    sprintf(buf, "Logical: [%d, %d] %s | Visual: [%.1f, %.1f] %.0f\nEnemies: %d Active | %d Killed", 
            logicalX, logicalY, dirNames[logicalDir], camX, camY, camAngle, activeCount, enemiesKilled);
    
    RECT textRc = {10, 10, 800, 100};
    DrawTextA(hdc, buf, -1, &textRc, DT_LEFT | DT_TOP);
    
    const char* help = "WASD - Move/Turn  |  SPACE - Dash  |  Q - Quit";
    TextOutA(hdc, 10, H - 30, help, (int)strlen(help));
}

// ── Input & Movement ───────────────────────────────────────────────────────
void ResetGame() {
    logicalX = 1; logicalY = 1;
    tgtX = 1.5; tgtY = 1.5;
    enemiesKilled = 0;
    
    // Revive and reset stalkers for PoC
    for (auto& e : entities) { e.active = false; }
    
    if(entities.size() >= 1) { entities[0].active = true; entities[0].x = 5; entities[0].y = 2; entities[0].type = ENEMY_STALKER; }
    if(entities.size() >= 2) { entities[1].active = true; entities[1].x = 3; entities[1].y = 1; entities[1].type = ENEMY_PATROL; entities[1].dir = 1; }
    if(entities.size() >= 3) { entities[2].active = true; entities[2].x = 7; entities[2].y = 5; entities[2].type = ENEMY_GLITCH; }
}

int distMap[MH][MW];
void computePlayerDistMap() {
    for (int y=0; y<MH; ++y) for (int x=0; x<MW; ++x) distMap[y][x] = 9999;
    
    struct Node { int x, y; };
    std::vector<Node> q;
    int head = 0;
    
    q.push_back({logicalX, logicalY});
    distMap[logicalY][logicalX] = 0;
    
    int dxArr[4] = {0, 1, 0, -1};
    int dyArr[4] = {-1, 0, 1, 0};
    
    while(head < q.size()) {
        Node curr = q[head++];
        int d = distMap[curr.y][curr.x];
        
        for (int i=0; i<4; i++) {
            int nx = curr.x + dxArr[i];
            int ny = curr.y + dyArr[i];
            
            if (nx >= 0 && nx < MW && ny >= 0 && ny < MH && maze[ny][nx] == 0) {
                if (distMap[ny][nx] > d + 1) {
                    distMap[ny][nx] = d + 1;
                    q.push_back({nx, ny});
                }
            }
        }
    }
}

void UpdateEntities() {
    // 1. Did player walk into an enemy without dashing?
    if (!isDashing) {
        for (const auto& e : entities) {
            if (e.active && e.x == logicalX && e.y == logicalY) {
                ResetGame();
                return;
            }
        }
    }

    // 2. AI Turn
    computePlayerDistMap(); // Generate perfect chase map
    
    for (auto& e : entities) {
        if (!e.active) continue;
        
        if (e.type == ENEMY_STALKER) {
            int dxArr[4] = {  0,  1,  0, -1 };
            int dyArr[4] = { -1,  0,  1,  0 };
            
            int bestDist = distMap[e.y][e.x];
            int bestX = e.x, bestY = e.y;
            
            // Look for neighbor closest to player
            for (int i=0; i<4; ++i) {
                int nx = e.x + dxArr[i];
                int ny = e.y + dyArr[i];
                if (nx >= 0 && nx < MW && ny >= 0 && ny < MH && maze[ny][nx] == 0) {
                    bool occupied = false;
                    for (const auto& oe : entities) {
                        if (oe.active && &oe != &e && oe.x == nx && oe.y == ny) occupied = true;
                    }
                    if (!occupied && distMap[ny][nx] < bestDist) {
                        bestDist = distMap[ny][nx];
                        bestX = nx;
                        bestY = ny;
                    }
                }
            }
            e.x = bestX;
            e.y = bestY;
        } 
        else if (e.type == ENEMY_PATROL) {
            int dxArr[4] = {  0,  1,  0, -1 };
            int dyArr[4] = { -1,  0,  1,  0 };
            
            int nx = e.x + dxArr[e.dir];
            int ny = e.y + dyArr[e.dir];
            
            bool blocked = (nx < 0 || nx >= MW || ny < 0 || ny >= MH || maze[ny][nx] == 1);
            for(const auto& oe : entities) {
                if(oe.active && &oe != &e && oe.x == nx && oe.y == ny) blocked = true;
            }
            
            if (blocked) {
                e.dir = (e.dir + 2) % 4; // Turn around 180 degrees
                nx = e.x + dxArr[e.dir];
                ny = e.y + dyArr[e.dir];
                blocked = (nx < 0 || nx >= MW || ny < 0 || ny >= MH || maze[ny][nx] == 1);
                for(const auto& oe : entities) {
                    if(oe.active && &oe != &e && oe.x == nx && oe.y == ny) blocked = true;
                }
            }
            
            if (!blocked) {
                e.x = nx; e.y = ny;
            }
        }
        else if (e.type == ENEMY_GLITCH) {
            // Glitch is stationary until player is near, then pulses or triggers
            // For now, it's just a blocking obstacle that resets game if touched
        }
    }
    
    // 3. Did any enemy walk into the player?
    if (!isDashing) {
        for (const auto& e : entities) {
            if (e.active && e.x == logicalX && e.y == logicalY) {
                ResetGame();
                return;
            }
        }
    } else {
        // We are dashing, so if an enemy walked onto us during its turn, it dies!
        for (auto& e : entities) {
            if (e.active && e.x == logicalX && e.y == logicalY) {
                e.active = false;
                enemiesKilled++;
            }
        }
    }
    
    // 4. Spawner (Spawn up to 7 Stalkers occasionally)
    int activeStalkers = 0;
    for (const auto& e : entities) {
        if (e.active && e.type == ENEMY_STALKER) activeStalkers++;
    }
    
    // Every AI tick, 10% chance to spawn if under limit
    if (activeStalkers < 7 && (rand() % 100 < 10)) {
        int sx = rand() % MW;
        int sy = rand() % MH;
        if (maze[sy][sx] == 0) {
            // Check distance from player to avoid cheap spawns
            int dist = abs(sx - logicalX) + abs(sy - logicalY);
            if (dist > 5) {
                bool occupied = false;
                for (const auto& e : entities) {
                    if (e.active && e.x == sx && e.y == sy) { occupied = true; break; }
                }
                if (!occupied) {
                    // Find inactive entity to recycle, or expand vector
                    bool spawned = false;
                    for (auto& e : entities) {
                        if (!e.active) {
                            e.type = ENEMY_STALKER; e.x = sx; e.y = sy; e.active = true;
                            spawned = true; break;
                        }
                    }
                    if (!spawned) {
                        entities.push_back({ENEMY_STALKER, sx, sy, 0, true});
                    }
                }
            }
        }
    }
}

void revealLineOfSight() {
    int dxArr[4] = {  0,  1,  0, -1 };
    int dyArr[4] = { -1,  0,  1,  0 };
    int maxDist = 6; // Sight range
    visited[logicalY][logicalX] = 1;
    
    // Check line of sight ahead
    for(int i=1; i<=maxDist; i++) {
        int nx = logicalX + dxArr[logicalDir] * i;
        int ny = logicalY + dyArr[logicalDir] * i;
        if(nx >= 0 && nx < MW && ny >= 0 && ny < MH) {
            visited[ny][nx] = 1;
            // Also reveal left and right walls of corridor
            if (logicalDir == 0 || logicalDir == 2) {
                if(nx-1 >= 0) visited[ny][nx-1] = 1;
                if(nx+1 < MW) visited[ny][nx+1] = 1;
            } else {
                if(ny-1 >= 0) visited[ny-1][nx] = 1;
                if(ny+1 < MH) visited[ny+1][nx] = 1;
            }
            if(maze[ny][nx] == 1) break; // block sight
        } else {
            break;
        }
    }
}

void TryMove(int forwardDelta, bool dash = false) {
    int dxArr[4] = {  0,  1,  0, -1 };
    int dyArr[4] = { -1,  0,  1,  0 };
    
    int dist = dash ? 2 : 1;
    if (forwardDelta < 0) dist = 1; // Cannot dash backwards
    
    int finalNx = logicalX;
    int finalNy = logicalY;
    
    // Raycast movement to prevent jumping *through* walls
    for (int step = 1; step <= dist; step++) {
        int nx = logicalX + dxArr[logicalDir] * (step * (forwardDelta > 0 ? 1 : -1));
        int ny = logicalY + dyArr[logicalDir] * (step * (forwardDelta > 0 ? 1 : -1));
        
        if (nx >= 0 && nx < MW && ny >= 0 && ny < MH && maze[ny][nx] == 0) {
            finalNx = nx;
            finalNy = ny;
            
            // If dashing, destroy enemies in our path
            if (dash) {
                for (auto& e : entities) {
                    if (e.active && e.x == finalNx && e.y == finalNy) {
                        e.active = false; // Destroyed!
                        enemiesKilled++;
                    }
                }
            }
        } else {
            break; // Hit a wall, stop dashing early
        }
    }
    
    // Process Turn if player actually moved
    if (finalNx != logicalX || finalNy != logicalY) {
        logicalX = finalNx;
        logicalY = finalNy;
        tgtX = logicalX + 0.5;
        tgtY = logicalY + 0.5;
        
        if (dash && dist > 1) {
            isDashing = true;
            currentLerpSpeed = LERP_DASH;
        } else {
            currentLerpSpeed = LERP_NORMAL;
            isDashing = false;
        }
        
        revealLineOfSight();
        UpdateEntities();
    }
}

// ── Window Proc ────────────────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, TIMER_ID, 1000 / FPS, NULL);
        logicalX = 1; logicalY = 1; logicalDir = 1; // Start pos
        tgtX = camX = logicalX + 0.5;
        tgtY = camY = logicalY + 0.5;
        tgtAngle = camAngle = logicalDir * 90.0;
        
        entities.clear();
        entities.push_back({ENEMY_STALKER, 5, 2, 0, true});
        entities.push_back({ENEMY_PATROL, 3, 1, 1, true}); // Dir 1 = EAST
        entities.push_back({ENEMY_GLITCH, 7, 5, 0, true});
        
        revealLineOfSight();
        srand(GetTickCount());
        return 0;

    case WM_TIMER:
        // Math.Lerp
        camX = lerp(camX, tgtX, currentLerpSpeed);
        camY = lerp(camY, tgtY, currentLerpSpeed);
        
        // Return to normal speed as we arrive at destination
        if (abs(camX - tgtX) < 0.05 && abs(camY - tgtY) < 0.05) {
            currentLerpSpeed = LERP_NORMAL;
            isDashing = false;
        }
        
        // Nearest angle wrapping
        {
            double diff = tgtAngle - camAngle;
            while (diff > 180.0) diff -= 360.0;
            while (diff < -180.0) diff += 360.0;
            camAngle += diff * currentLerpSpeed;
            camAngle = angleWrap(camAngle);
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_KEYDOWN:
        switch (wp) {
        case 'W': TryMove(1);  break;
        case 'S': TryMove(-1); break;
        case VK_SPACE: TryMove(1, true); break; // DASH
        case 'A':
            logicalDir = (logicalDir + 3) % 4;
            tgtAngle = logicalDir * 90.0;
            revealLineOfSight();
            break;
        case 'D':
            logicalDir = (logicalDir + 1) % 4;
            tgtAngle = logicalDir * 90.0;
            revealLineOfSight();
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

        FillRect(memDC, &rc, blackBrush);
        
        // Generate and draw world
        render3D(memDC);
        drawMinimap(memDC);
        drawHUD(memDC);

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

// ── Entry ──────────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmd) {
    greenPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
    dimPen   = CreatePen(PS_SOLID, 2, RGB(0, 150, 0));
    darkPen  = CreatePen(PS_SOLID, 1, RGB(0, 70, 0));
    redPen   = CreatePen(PS_SOLID, 2, RGB(255, 50, 50));
    bluePen  = CreatePen(PS_SOLID, 2, RGB(50, 150, 255));
    blackBrush = CreateSolidBrush(RGB(0, 0, 0));

    WNDCLASSEXA wc = {};
    wc.cbSize       = sizeof(wc);
    wc.lpfnWndProc  = WndProc;
    wc.hInstance    = hInst;
    wc.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName= "MazeV2Class";
    wc.hbrBackground= blackBrush;
    RegisterClassExA(&wc);

    RECT wr = { 0, 0, W, H };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExA(0, "MazeV2Class", "Wireframe Maze Engine (Phase 4) - Roguelike Stalker MVP",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInst, NULL);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    
    DeleteObject(greenPen);
    DeleteObject(dimPen);
    DeleteObject(darkPen);
    DeleteObject(redPen);
    DeleteObject(bluePen);
    DeleteObject(blackBrush);
    return (int)msg.wParam;
}