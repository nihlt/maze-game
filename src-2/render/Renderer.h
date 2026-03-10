#ifndef RENDERER_H
#define RENDERER_H

#include "GameTypes.h"

struct RenderResources {
    HPEN greenPen, dimPen, darkPen, redPen, bluePen, yellowPen;
    HBRUSH blackBrush;
};

void InitResources(RenderResources& res);
void DeleteResources(RenderResources& res);
void Render3D(HDC hdc, const GameState& gs, const RenderResources& res);
void DrawMinimap(HDC hdc, const GameState& gs, const RenderResources& res);
void DrawHUD(HDC hdc, const GameState& gs);
void DrawArchitect(HDC hdc, const GameState& gs, const RenderResources& res);

#endif // RENDERER_H
