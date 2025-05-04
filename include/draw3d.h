#ifndef DRAW3D_H
#define DRAW3D_H

#include "../include/vec3d.h"

void DRAW3D_DrawTriangle(int x1,int y1, int x2,int y2, int x3, int y3, SDL_Renderer *pRend);

void DRAW3D_CubeRender(
    SDL_Renderer *pRend,
    MeshCube      cube,
    Matrix4x4     matRotZ,
    Matrix4x4     matRotX,
    Matrix4x4     matProj
);

void VEC3D_Matrix4x4RotateX(Matrix4x4* mat, float theta);
void VEC3D_Matrix4x4RotateZ(Matrix4x4* mat, float theta);

#endif 
