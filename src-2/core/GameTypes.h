#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "Constants.h"
#include <vector>
#include <string>

// ── Entities ───────────────────────────────────────────────────────────────
enum EntityType { ENEMY_STALKER, ENEMY_PATROL, ENEMY_GLITCH, ENTITY_KEY, ENTITY_PORTAL };

struct Entity {
    EntityType type;
    int x, y;
    int dir; // For patrols
    bool active;
};

// ── Game State ──────────────────────────────────────────────────────────────
struct GameState {
    int maze[MH][MW];
    int visited[MH][MW];
    bool showFullMap; // Dev mode flag
    int currentLevel;
    int baseMazeSize; // Actual size used (can be <= MW/MH)
    
    int logicalX, logicalY;
    int logicalDir; // 0=N, 1=E, 2=S, 3=W

    // Camera Visual State (Interpolated)
    double camX, camY;
    double camAngle;
    double tgtX, tgtY;
    double tgtAngle;
    double currentLerpSpeed;
    bool isDashing;

    std::vector<Entity> entities;
    int enemiesKilled;
    int keysCollected;
    int totalKeys;

    // Architect State
    enum ArchState { HIDDEN, APPEARING, TYPING, WAITING, DISAPPEARING };

    struct ArchitectState {
        ArchState state;
        float yOffset; // For appearing/disappearing animation (0 = top, 1 = normal)
        int waitTimer;

        int anger;    // 1-13
        int aggress;  // 1-13
        int malice;   // 1-13

        // Interpolated values
        int curr_r, curr_g, curr_b;
        int curr_spd;

        // Rotation angles
        int angX, angY, angZ;

        // Step 2: Text Mechanics
        std::string targetText;
        std::string currentText;
        int textCharDelay;
        int textTimer;

        // Feedback 1: Inertial Text
        int sentenceStartIdx;
        int sentenceEndIdx;
        float currentSpeed; // chars per frame or similar
    } architect;
};

#endif // GAME_TYPES_H
