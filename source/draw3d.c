#include "../include/draw3d.h"

void DRAW3D_DrawTriangle(int x1,int y1, int x2,int y2, int x3,int y3, SDL_Renderer *pRend) {
    SDL_SetRenderDrawColor(pRend, 255,255,255,255);
    SDL_RenderDrawLine(pRend, x1,y1, x2,y2);
    SDL_RenderDrawLine(pRend, x2,y2, x3,y3);
    SDL_RenderDrawLine(pRend, x3,y3, x1,y1);
}

void DRAW3D_CubeRender(
    SDL_Renderer *pRend,
    MeshCube      cube,
    Matrix4x4     matRotZ,
    Matrix4x4     matRotX,
    Matrix4x4     matProj
) {
    // 1) pull out your triangles
    int w, h;
    SDL_GetRendererOutputSize(pRend, &w, &h);
    TriangleVector tv    = VEC3D_GetCubeTriangles(cube);
    size_t         count = VEC3D_TriangleVectorSize(tv);
    const Triangle *tris = VEC3D_TriangleVectorData(tv);

    // 2) for each triangle, do Z‑rotate, X‑rotate, translate, project, draw
    for (size_t i = 0; i < count; i++) {
        Triangle in        = tris[i];
        Triangle rz, rxz, trans, proj;

        // rotate Z
        VEC3D_Matrix4x4MultiplyVector(&in.p[0], &rz.p[0], &matRotZ);
        VEC3D_Matrix4x4MultiplyVector(&in.p[1], &rz.p[1], &matRotZ);
        VEC3D_Matrix4x4MultiplyVector(&in.p[2], &rz.p[2], &matRotZ);

        // rotate X
        VEC3D_Matrix4x4MultiplyVector(&rz.p[0], &rxz.p[0], &matRotX);
        VEC3D_Matrix4x4MultiplyVector(&rz.p[1], &rxz.p[1], &matRotX);
        VEC3D_Matrix4x4MultiplyVector(&rz.p[2], &rxz.p[2], &matRotX);

        // translate forward
        trans = rxz;
        trans.p[0].z += 3.0f;
        trans.p[1].z += 3.0f;
        trans.p[2].z += 3.0f;

        // project into 2D
        VEC3D_Matrix4x4MultiplyVector(&trans.p[0], &proj.p[0], &matProj);
        VEC3D_Matrix4x4MultiplyVector(&trans.p[1], &proj.p[1], &matProj);
        VEC3D_Matrix4x4MultiplyVector(&trans.p[2], &proj.p[2], &matProj);

        SDL_GetRendererOutputSize(pRend, &w, &h);
        for (int j = 0; j < 3; j++) {
            proj.p[j].x = (proj.p[j].x + 1.0f) * 0.5f * w;
            proj.p[j].y = (proj.p[j].y + 1.0f) * 0.5f * h;
        }

        // finally draw it
        DRAW3D_DrawTriangle(
            (int)proj.p[0].x, (int)proj.p[0].y,
            (int)proj.p[1].x, (int)proj.p[1].y,
            (int)proj.p[2].x, (int)proj.p[2].y,
            pRend
        );
    }
}