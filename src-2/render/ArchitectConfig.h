#ifndef ARCHITECT_CONFIG_H
#define ARCHITECT_CONFIG_H

#include <stdint.h>

// Architect Emotion Engine Configuration
namespace ArchitectConfig {
    // ── Emotion Level Bounds ────────────────────────────────────────────────
    const int MIN_LEVEL = 1;
    const int MAX_LEVEL = 13;

    // ── Color Gradients (1..13) ─────────────────────────────────────────────
    // Format: RGB for each level. The 0-th index is dummy to match level index.
    const uint8_t COL_R[] = {0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0xFF, 0xFF, 0xFF, 0xFF, 0xCC, 0xAA, 0x88};
    const uint8_t COL_G[] = {0, 0x00, 0x88, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0x55, 0x00, 0x00, 0x00, 0x00};
    const uint8_t COL_B[] = {0, 0xFF, 0xFF, 0xFF, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0xAA, 0xFF};

    // ── Rotation Speed (Target * 256 for interpolation) ──────────────────────
    // 1..13 levels of aggressiveness
    const int32_t SPD_TGT[] = {0, 256, 307, 384, 460, 563, 665, 793, 921, 1075, 1228, 1408, 1700, 2048};

    // ── Smoothing Divisors ──────────────────────────────────────────────────
    const int COLOR_SMOOTH_DIVISOR = 8;
    const int SPEED_SMOOTH_DIVISOR = 16;

    // ── Scale Pulsation Configuration ───────────────────────────────────────
    const int MALICE_THRESHOLD = 8;    // Pulsation starts when malice > threshold
    const float PULSE_AMPLITUDE = 0.1f; // Intensity of scale change per malice point above threshold
    const int BASE_SCALE = 35;         // Cube scale on HUD
    const int RENDER_DIST = 200;       // Perspective distance
    
    // ── Positioning ─────────────────────────────────────────────────────────
    const int HUD_Y_OFFSET = 70;      // Vertical position on screen

    // ── Text Highlight Dictionary ──────────────────────────────────────────
    const char* const HIGHLIGHT_WORDS[] = { "ARCHITECT", "MAZE", "ESCAPE", "SEE" };
    const int HIGHLIGHT_COUNT = 4;
    const uint32_t HIGHLIGHT_COLOR = 0x0000FFFF; // Yellow (BGR for GDI or RGB depending on use, let's use RGB(255,255,0))

    // ── Rainbow Highlight Configuration ───────────────────────────────────
    const float RAINBOW_SPEED_MULT = 0.02f; // Multiplier for cube rotation speed to rainbow speed
    const float RAINBOW_LETTER_OFFSET = 0.2f; // Offset in hue between consecutive letters

    // ── Portal Configuration ────────────────────────────────────────────────
    const uint8_t PORTAL_R = 0;
    const uint8_t PORTAL_G = 150;
    const uint8_t PORTAL_B = 255;
    const uint8_t PORTAL_ALPHA = 100; // ~40% transparency (0-255)
}

#endif // ARCHITECT_CONFIG_H
