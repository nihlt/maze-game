#include "Renderer.h"

void InitResources(RenderResources& res) {
    res.greenPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
    res.dimPen   = CreatePen(PS_SOLID, 2, RGB(0, 150, 0));
    res.darkPen  = CreatePen(PS_SOLID, 1, RGB(0, 70, 0));
    res.redPen   = CreatePen(PS_SOLID, 2, RGB(255, 50, 50));
    res.bluePen  = CreatePen(PS_SOLID, 2, RGB(50, 150, 255));
    res.yellowPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));
    res.blackBrush = CreateSolidBrush(RGB(0, 0, 0));
}

void DeleteResources(RenderResources& res) {
    DeleteObject(res.greenPen);
    DeleteObject(res.dimPen);
    DeleteObject(res.darkPen);
    DeleteObject(res.redPen);
    DeleteObject(res.bluePen);
    DeleteObject(res.yellowPen);
    DeleteObject(res.blackBrush);
}
