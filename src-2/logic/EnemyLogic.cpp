#include "EnemyLogic.h"
#include "GameLogic.h"
#include <algorithm>
#include <cstdlib>
#include <vector>

void ComputePlayerDistMap(const GameState& gs, int distMap[MH][MW]) {
    for (int y=0; y<MH; ++y) for (int x=0; x<MW; ++x) distMap[y][x] = 9999;
    
    struct Node { int x, y; };
    std::vector<Node> q;
    int head = 0;
    
    q.push_back({gs.logicalX, gs.logicalY});
    distMap[gs.logicalY][gs.logicalX] = 0;
    
    int dxArr[4] = {0, 1, 0, -1};
    int dyArr[4] = {-1, 0, 1, 0};
    
    while(head < (int)q.size()) {
        Node curr = q[head++];
        int d = distMap[curr.y][curr.x];
        
        for (int i=0; i<4; i++) {
            int nx = curr.x + dxArr[i];
            int ny = curr.y + dyArr[i];
            
            if (nx >= 0 && nx < MW && ny >= 0 && ny < MH && gs.maze[ny][nx] == 0) {
                if (distMap[ny][nx] > d + 1) {
                    distMap[ny][nx] = d + 1;
                    q.push_back({nx, ny});
                }
            }
        }
    }
}

bool IsEnemy(EntityType type) {
    return type == ENEMY_STALKER || type == ENEMY_PATROL || type == ENEMY_GLITCH;
}

void UpdateEntities(GameState& gs) {
    // 1. Did player walk into an enemy without dashing?
    if (!gs.isDashing) {
        for (const auto& e : gs.entities) {
            if (e.active && IsEnemy(e.type) && e.x == gs.logicalX && e.y == gs.logicalY) {
                ResetGame(gs);
                return;
            }
        }
    }

    // 2. AI Turn
    int distMap[MH][MW];
    ComputePlayerDistMap(gs, distMap); // Generate perfect chase map
    
    for (auto& e : gs.entities) {
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
                if (nx >= 0 && nx < MW && ny >= 0 && ny < MH && gs.maze[ny][nx] == 0) {
                    bool occupied = false;
                    for (const auto& oe : gs.entities) {
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
            
            bool blocked = (nx < 0 || nx >= MW || ny < 0 || ny >= MH || gs.maze[ny][nx] == 1);
            for(const auto& oe : gs.entities) {
                if(oe.active && &oe != &e && oe.x == nx && oe.y == ny) blocked = true;
            }
            
            if (blocked) {
                e.dir = (e.dir + 2) % 4; // Turn around 180 degrees
                nx = e.x + dxArr[e.dir];
                ny = e.y + dyArr[e.dir];
                blocked = (nx < 0 || nx >= MW || ny < 0 || ny >= MH || gs.maze[ny][nx] == 1);
                for(const auto& oe : gs.entities) {
                    if(oe.active && &oe != &e && oe.x == nx && oe.y == ny) blocked = true;
                }
            }
            
            if (!blocked) {
                e.x = nx; e.y = ny;
            }
        }
    }
    
    // 3. Did any enemy walk into the player?
    if (!gs.isDashing) {
        for (const auto& e : gs.entities) {
            if (e.active && IsEnemy(e.type) && e.x == gs.logicalX && e.y == gs.logicalY) {
                ResetGame(gs);
                return;
            }
        }
    } else {
        // We are dashing, so if an enemy walked onto us during its turn, it dies!
        for (auto& e : gs.entities) {
            if (e.active && e.x == gs.logicalX && e.y == gs.logicalY) {
                e.active = false;
                gs.enemiesKilled++;
            }
        }
    }
    
    // 4. Spawner
    int activeStalkers = 0;
    for (const auto& e : gs.entities) {
        if (e.active && e.type == ENEMY_STALKER) activeStalkers++;
    }
    
    if (activeStalkers < 7 && (rand() % 100 < 10)) {
        int sx = rand() % MW;
        int sy = rand() % MH;
        if (gs.maze[sy][sx] == 0) {
            int dist = abs(sx - gs.logicalX) + abs(sy - gs.logicalY);
            if (dist > 5) {
                bool occupied = false;
                for (const auto& e : gs.entities) {
                    if (e.active && e.x == sx && e.y == sy) { occupied = true; break; }
                }
                if (!occupied) {
                    bool spawned = false;
                    for (auto& e : gs.entities) {
                        if (!e.active) {
                            e.type = ENEMY_STALKER; e.x = sx; e.y = sy; e.active = true;
                            spawned = true; break;
                        }
                    }
                    if (!spawned) {
                        gs.entities.push_back({ENEMY_STALKER, (int)sx, (int)sy, 0, true});
                    }
                }
            }
        }
    }
}
