#ifndef DRAW3D_H
#define DRAW3D_H

#include "../include/matrix.h"

void DRAW3D_DrawTriangle(int x1,int y1, int x2,int y2, int x3, int y3, SDL_Renderer *pRend,uint8_t shade);
void DRAW3D_DrawFillTriangle(SDL_Renderer* pRend, SDL_Point v0, SDL_Point v1, SDL_Point v2, uint8_t shade);
void DRAW3D_Shutdown(void);

void DRAW3D_MeshRender(
    SDL_Renderer*   pRend,
    TriangleVector  mesh,
    Matrix4x4       matWorld,
    Matrix4x4       matView,
    Matrix4x4       matProj,
    Vec3d           vCamera,
    SDL_Texture*    tex
);

void TexturedTriangle(	
    int x1, int y1, float u1, float v1, float w1,
    int x2, int y2, float u2, float v2, float w2,
    int x3, int y3, float u3, float v3, float w3,
    SDL_Surface *texSurf, SDL_Renderer *pRend, float *pDepthBuffer
);

void TexturedTriangle_GPU(
    int x1, int y1, float u1, float v1,
    int x2, int y2, float u2, float v2,
    int x3, int y3, float u3, float v3,
    SDL_Texture *tex, SDL_Renderer *r, 
    uint8_t shade
);

#endif 

