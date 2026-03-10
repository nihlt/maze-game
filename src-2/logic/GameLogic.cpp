#include "GameLogic.h"
#include "EnemyLogic.h"
#include "MazeGenerator.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <random>

void ResetGame(GameState& gs) {
    // 1. Level & Size scaling
    // If it's the first run, currentLevel might be 0 or uninitialized.
    // Let's assume gs is zero-initialized or we handle it here.
    if (gs.currentLevel <= 0) gs.currentLevel = 1;
    
    // Example: level 1 is 11x11, level 2 is 13x13 (if MW/MH allow)
    // We must stay within MW and MH limits.
    int size = 9 + (gs.currentLevel * MAZE_SIZE_INCREMENT);
    if (size > MW) size = MW;
    if (size > MH) size = MH;
    // The algorithm works best with odd dimensions
    if (size % 2 == 0) size--;
    if (size < 5) size = 5;

    // Maze generator uses MH/MW for array bounds, but we can fill it with walls outside 'size'
    GenerateMaze(gs.maze, gs.logicalX, gs.logicalY, size, size);
    
    // Fill areas outside current 'size' with walls to keep player contained
    for(int y=0; y<MH; y++) {
        for(int x=0; x<MW; x++) {
            if(x >= size || y >= size) gs.maze[y][x] = 1;
            gs.visited[y][x] = 0;
        }
    }

    gs.tgtX = gs.logicalX + 0.5; gs.tgtY = gs.logicalY + 0.5;

    // Fix spawn orientation: look towards the open path
    if (gs.logicalX + 1 < size && gs.maze[gs.logicalY][gs.logicalX + 1] == 0) {
        gs.logicalDir = 1; // East
    } else if (gs.logicalY + 1 < size && gs.maze[gs.logicalY + 1][gs.logicalX] == 0) {
        gs.logicalDir = 2; // South
    } else if (gs.logicalX - 1 >= 0 && gs.maze[gs.logicalY][gs.logicalX - 1] == 0) {
        gs.logicalDir = 3; // West
    } else {
        gs.logicalDir = 0; // North
    }
    gs.tgtAngle = gs.logicalDir * 90.0;
    gs.camAngle = gs.tgtAngle;

    gs.enemiesKilled = 0;
    gs.keysCollected = 0;
    gs.totalKeys = 0;
    
    // 2. Random Spawning
    // Clear old entities
    gs.entities.clear();
    
    // Find empty slots
    std::vector<std::pair<int, int>> emptySlots;
    for(int y=1; y<size-1; y++) {
        for(int x=1; x<size-1; x++) {
            if(gs.maze[y][x] == 0 && (x != gs.logicalX || y != gs.logicalY)) {
                emptySlots.push_back({x, y});
            }
        }
    }
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(emptySlots.begin(), emptySlots.end(), g);
    
    // Spawn some enemies and keys
    int numKeys = 3;
    int numEnemies = 3;
    
    int idx = 0;
    for(int i=0; i<numKeys && idx < emptySlots.size(); i++) {
        Entity e;
        e.type = ENTITY_KEY;
        e.x = emptySlots[idx].first;
        e.y = emptySlots[idx].second;
        e.active = true;
        gs.entities.push_back(e);
        gs.totalKeys++;
        idx++;
    }
    
    EntityType types[] = {ENEMY_STALKER, ENEMY_PATROL, ENEMY_GLITCH};
    for(int i=0; i<numEnemies && idx < emptySlots.size(); i++) {
        Entity e;
        e.type = types[i % 3];
        e.x = emptySlots[idx].first;
        e.y = emptySlots[idx].second;
        e.active = true;
        e.dir = rand() % 4;
        gs.entities.push_back(e);
        idx++;
    }

    // Spawn Portal
    if (idx < emptySlots.size()) {
        Entity e;
        e.type = ENTITY_PORTAL;
        e.x = emptySlots[idx].first;
        e.y = emptySlots[idx].second;
        e.active = true;
        gs.entities.push_back(e);
        idx++;
    }

    RevealLineOfSight(gs);

    // Initialize Architect
    gs.architect.state = GameState::HIDDEN;
    gs.architect.yOffset = 0;
    gs.architect.waitTimer = 0;

    TriggerArchitect(gs, 3, 3, 3, "WELCOME TO THE MAZE, SUBJECT. FIND ALL KEYS TO ESCAPE... IF YOU CAN.", 4000);
    gs.architect.anger = 3;
    gs.architect.aggress = 3;
    gs.architect.malice = 3;
    gs.architect.curr_r = 0;
    gs.architect.curr_g = 0x88;
    gs.architect.curr_b = 0xFF;
    gs.architect.curr_spd = 256;
    gs.architect.angX = 0;
    gs.architect.angY = 0;
    gs.architect.angZ = 0;
    gs.architect.targetText = "";
    gs.architect.currentText = "";
}

void TriggerArchitect(GameState& gs, int anger, int aggress, int malice, const std::string& text, int waitTimeMs) {
    if (gs.architect.state != GameState::HIDDEN) return; // Don't interrupt

    gs.architect.anger = anger;
    gs.architect.aggress = aggress;
    gs.architect.malice = malice;
    gs.architect.targetText = text;
    gs.architect.currentText = "";
    gs.architect.waitTimer = waitTimeMs / 16; // Approx frames (assuming 60fps)
    gs.architect.textTimer = 0;
    gs.architect.textCharDelay = 2; // Default delay
    gs.architect.state = GameState::APPEARING;
    gs.architect.yOffset = 0.0f;
}

void RevealLineOfSight(GameState& gs) {
    int dxArr[4] = {  0,  1,  0, -1 };
    int dyArr[4] = { -1,  0,  1,  0 };
    int maxDist = 6;
    gs.visited[gs.logicalY][gs.logicalX] = 1;
    
    for(int i=1; i<=maxDist; i++) {
        int nx = gs.logicalX + dxArr[gs.logicalDir] * i;
        int ny = gs.logicalY + dyArr[gs.logicalDir] * i;
        if(nx >= 0 && nx < MW && ny >= 0 && ny < MH) {
            gs.visited[ny][nx] = 1;
            if (gs.logicalDir == 0 || gs.logicalDir == 2) {
                if(nx-1 >= 0) gs.visited[ny][nx-1] = 1;
                if(nx+1 < MW) gs.visited[ny][nx+1] = 1;
            } else {
                if(ny-1 >= 0) gs.visited[ny-1][nx] = 1;
                if(ny+1 < MH) gs.visited[ny+1][nx] = 1;
            }
            if(gs.maze[ny][nx] == 1) break;
        } else {
            break;
        }
    }
}

void TryMove(GameState& gs, int forwardDelta, bool dash) {
    int dxArr[4] = {  0,  1,  0, -1 };
    int dyArr[4] = { -1,  0,  1,  0 };
    
    int dist = dash ? 2 : 1;
    if (forwardDelta < 0) dist = 1;
    
    int finalNx = gs.logicalX;
    int finalNy = gs.logicalY;
    
    for (int step = 1; step <= dist; step++) {
        int nx = gs.logicalX + dxArr[gs.logicalDir] * (step * (forwardDelta > 0 ? 1 : -1));
        int ny = gs.logicalY + dyArr[gs.logicalDir] * (step * (forwardDelta > 0 ? 1 : -1));
        
        if (nx >= 0 && nx < MW && ny >= 0 && ny < MH && gs.maze[ny][nx] == 0) {
            finalNx = nx;
            finalNy = ny;
            
            // Interaction with entities at the new position
            for (auto& e : gs.entities) {
                if (e.active && e.x == finalNx && e.y == finalNy) {
                    if (e.type == ENTITY_KEY) {
                        e.active = false;
                        gs.keysCollected++;
                        if (gs.keysCollected >= gs.totalKeys) {
                            TriggerArchitect(gs, 4, 3, 5, "THE PORTAL IS NOW OPEN. FIND IT AND PROCEED.", 3000);
                        } else {
                            TriggerArchitect(gs, 5, 4, 4, "A KEY... BUT YOU ARE STILL FAR FROM FREEDOM.", 3000);
                        }
                    } else if (e.type == ENTITY_PORTAL) {
                        if (gs.keysCollected >= gs.totalKeys) {
                            gs.currentLevel++;
                            ResetGame(gs);
                            return; // ResetGame handles new state
                        }
                    } else if (dash) {
                        e.active = false;
                        gs.enemiesKilled++;
                    }
                }
            }
        } else {
            break;
        }
    }
    
    if (finalNx != gs.logicalX || finalNy != gs.logicalY) {
        gs.logicalX = finalNx;
        gs.logicalY = finalNy;
        gs.tgtX = gs.logicalX + 0.5;
        gs.tgtY = gs.logicalY + 0.5;
        
        if (dash && dist > 1) {
            gs.isDashing = true;
            gs.currentLerpSpeed = LERP_DASH;
        } else {
            gs.currentLerpSpeed = LERP_NORMAL;
            gs.isDashing = false;
        }
        
        RevealLineOfSight(gs);
        UpdateEntities(gs);
    }
}
