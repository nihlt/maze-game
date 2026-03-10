#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

// Camera configuration
const double FOV_SCALE = 600.0;     // focal length / zoom
const double WALL_HEIGHT = 0.6;     // height from eye level
const double Z_NEAR = 0.1;          // Near clipping plane

const double LERP_NORMAL = 0.15;
const double LERP_DASH = 0.40;  // Faster visual transition for dash

const int MAZE_SIZE_INCREMENT = 2; // Increase by 2 to maintain odd dimensions

#endif // CONSTANTS_H
