#ifndef DRAW3D_H
#define DRAW3D_H

#include "../include/matrix4x4.h"

void DRAW3D_DrawTriangle(int x1,int y1, int x2,int y2, int x3, int y3, SDL_Renderer *pRend,uint8_t shade);
void DRAW3D_DrawFillTriangle(SDL_Renderer* pRend, SDL_Point v0, SDL_Point v1, SDL_Point v2, uint8_t shade);

void DRAW3D_MeshRender(
    SDL_Renderer*   pRend,
    TriangleVector  mesh,
    Matrix4x4       matWorld,
    Matrix4x4       matProj,
    Vec3d           vCamera
);
#endif 

