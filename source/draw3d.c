#include "../include/draw3d.h"    
#include <stdio.h>                
#include <stdint.h>               
#include <stddef.h>               


void DRAW3D_DrawTriangle(int x1,int y1, int x2,int y2, int x3,int y3, SDL_Renderer *pRend, uint8_t shade){
    SDL_SetRenderDrawColor(pRend, shade,shade,shade,255);
    SDL_RenderDrawLine(pRend, x1,y1, x2,y2);
    SDL_RenderDrawLine(pRend, x2,y2, x3,y3);
    SDL_RenderDrawLine(pRend, x3,y3, x1,y1);
    SDL_SetRenderDrawColor(pRend, 255,255,255,255);
}

void DRAW3D_DrawFillTriangle(SDL_Renderer* pRend, SDL_Point v0, SDL_Point v1, SDL_Point v2, uint8_t shade){
    SDL_SetRenderDrawColor(pRend,shade,shade,shade,255);
    if (v1.y < v0.y) { SDL_Point t = v0; v0 = v1; v1 = t; }
    if (v2.y < v0.y) { SDL_Point t = v0; v0 = v2; v2 = t; }
    if (v2.y < v1.y) { SDL_Point t = v1; v1 = v2; v2 = t; }

    float dx02 = (float)(v2.x - v0.x) / (float)(v2.y - v0.y);

    float xl = (float)v0.x;
    float xr = (float)v0.x;

    if (v1.y > v0.y) {
        float dx01 = (float)(v1.x - v0.x) / (float)(v1.y - v0.y);
        xl = v0.x; 
        xr = v0.x;
        for (int y = v0.y; y <= v1.y; y++) {
            int xL = (int)(xl + 0.5f);
            int xR = (int)(xr + 0.5f);
            SDL_RenderDrawLine(pRend, xL, y, xR, y);
            xl += dx02;
            xr += dx01;
        }
    }
    if (v2.y > v1.y) {
        float dx12 = (float)(v2.x - v1.x) / (float)(v2.y - v1.y);
        // reset xl to the long edge at scanline v1.y
        xl = v0.x + dx02 * (float)(v1.y - v0.y);
        xr = v1.x;
        for (int y = v1.y; y <= v2.y; y++) {
            int xL = (int)(xl + 0.5f);
            int xR = (int)(xr + 0.5f);
            SDL_RenderDrawLine(pRend, xL, y, xR, y);
            xl += dx02;
            xr += dx12;
        }
    }
}

void DRAW3D_MeshRender(
    SDL_Renderer*    pRend,
    TriangleVector   mesh,
    Matrix4x4        matWorld,
    Matrix4x4        matView,
    Matrix4x4        matProj,
    Vec3d            vCamera){
    int w, h;
    SDL_GetRendererOutputSize(pRend, &w, &h);

    size_t count = VEC3D_TriangleVectorSize(mesh);
    const Triangle *tris = VEC3D_TriangleVectorData(mesh);
    TriangleVector sortTri = VEC3D_TriangleVectorCreate();
    for (size_t i = 0; i < count; i++) {
        Triangle in = tris[i];
        Triangle triTrans, triView;

        // World transform
        triTrans.p[0] = MATRIX_Matrix4x4MultiplyVector(&in.p[0], &matWorld);
        triTrans.p[1] = MATRIX_Matrix4x4MultiplyVector(&in.p[1], &matWorld);
        triTrans.p[2] = MATRIX_Matrix4x4MultiplyVector(&in.p[2], &matWorld);

        // Compute normal
        Vec3d line1 = VEC3D_Vec3dSub(&triTrans.p[1], &triTrans.p[0]);
        Vec3d line2 = VEC3D_Vec3dSub(&triTrans.p[2], &triTrans.p[0]);
        Vec3d cp    = VEC3D_Vec3dCrossProduct(&line1, &line2);
        Vec3d normal= VEC3D_Vec3dNormalize(&cp);

        // Back-face cull
        Vec3d viewRay = VEC3D_Vec3dSub(&triTrans.p[0], &vCamera);
        if (VEC3D_Vec3dDotProduct(&normal, &viewRay) < 0.0f) {
            Vec3d light_dir = {0.0f,0.0f,-1.0f,1.0f};
            light_dir = VEC3D_Vec3dNormalize(&light_dir);

            float dp_light = fmaxf(0.1f,VEC3D_Vec3dDotProduct(&normal,&light_dir));
            const float ambient = 0.1f;
            uint8_t shade = (uint8_t)((ambient + (1.0f-ambient)*dp_light)*255);
            triTrans.shade = shade;

            // View transform
            triView.p[0] = MATRIX_Matrix4x4MultiplyVector(&triTrans.p[0], &matView);
            triView.p[1] = MATRIX_Matrix4x4MultiplyVector(&triTrans.p[1], &matView);
            triView.p[2] = MATRIX_Matrix4x4MultiplyVector(&triTrans.p[2], &matView);
            triView.shade = triTrans.shade;
            // Clip against near plane
            Triangle clipped[2];
            int nClipped = 0;
            nClipped = VEC3D_ClipAgainstPlane((Vec3d){0,0,0.1f,1}, (Vec3d){0,0,1.0f,1.0f}, &triView, &clipped[0], &clipped[1]);
            for (int j = 0; j < nClipped; j++) {
                // Project
                Vec3d p0 = MATRIX_Matrix4x4MultiplyVector(&clipped[j].p[0], &matProj);
                Vec3d p1 = MATRIX_Matrix4x4MultiplyVector(&clipped[j].p[1], &matProj);
                Vec3d p2 = MATRIX_Matrix4x4MultiplyVector(&clipped[j].p[2], &matProj);
                if (p0.w<=0||p1.w<=0||p2.w<=0) continue;

                Triangle proj;
                proj.p[0] = VEC3D_Vec3dDiv(&p0,p0.w); proj.p[0].w=1;
                proj.p[1] = VEC3D_Vec3dDiv(&p1,p1.w); proj.p[1].w=1;
                proj.p[2] = VEC3D_Vec3dDiv(&p2,p2.w); proj.p[2].w=1;
                proj.shade = clipped[j].shade;
                // Screen coords
                for (int k=0;k<3;k++) {
                    proj.p[k].x *= -1.0f; 
                    proj.p[k].y *= -1.0f;
                }
                Vec3d vOffsetView = VEC3D_Vec3dConstructor(1,1,0);
                proj.p[0] = VEC3D_Vec3dAdd(&proj.p[0], &vOffsetView);
                proj.p[1] = VEC3D_Vec3dAdd(&proj.p[1], &vOffsetView);
                proj.p[2] = VEC3D_Vec3dAdd(&proj.p[2], &vOffsetView);
                for (int k = 0;k < 3; k++){
                    proj.p[k].x *= 0.5f * (float)w;
                    proj.p[k].y *= 0.5f * (float)h;
                }
                
                VEC3D_TriangleVectorPush(sortTri, proj);                
            }
        }
    }

    // Sort & draw
    VEC3D_TriangleVectorSortByMidZ(sortTri);
    for (size_t ti = 0; ti < VEC3D_TriangleVectorSize(sortTri); ti++) {
        // 1) start with exactly one triangle
        Triangle t0 = VEC3D_TriangleVectorGetAt(sortTri, ti);
        TriangleVector list = VEC3D_TriangleVectorCreate();
        VEC3D_TriangleVectorPush(list, t0);

        // 2) define the four screen‐edge planes
        Vec3d planePnts[4] = {
            { 0, 0, 0, 1.0f },   // top edge y =   0
            { 0, (float)h-1.0f, 0, 1.0f },   // bottom y = h–1
            { 0, 0, 0, 1.0f },   // left   x =   0
            { (float)w-1.0f ,0, 0, 1.0f }    // right  x = w–1
        };
        Vec3d planeNrms[4] = {
            {  0,  1.0f, 0, 1.0f },
            {  0, -1.0f, 0, 1.0f },
            {  1.0f,  0, 0, 1.0f },
            { -1.0f,  0, 0, 1.0f }
        };

        // 3) clip against each plane in turn
        for (int p = 0; p < 4; p++) {
            TriangleVector output = VEC3D_TriangleVectorCreate();
            size_t cnt = VEC3D_TriangleVectorSize(list);
            for (size_t j = 0; j < cnt; j++) {
                Triangle in = VEC3D_TriangleVectorGetAt(list, j);
                Triangle outT[2];
                int nClipped = VEC3D_ClipAgainstPlane(
                    planePnts[p], planeNrms[p],
                    &in, &outT[0], &outT[1]
                );
                for (int k = 0; k < nClipped; k++)
                    VEC3D_TriangleVectorPush(output, outT[k]);
            }
            VEC3D_TriangleVectorDestroy(list);
            list = output;
        }

        // 4) whatever remains is guaranteed inside the window—draw it
        for (size_t j = 0; j < VEC3D_TriangleVectorSize(list); j++) {
            Triangle t = VEC3D_TriangleVectorGetAt(list, j);
            DRAW3D_DrawFillTriangle(
                pRend,
                (SDL_Point){(int)t.p[0].x, (int)t.p[0].y},
                (SDL_Point){(int)t.p[1].x, (int)t.p[1].y},
                (SDL_Point){(int)t.p[2].x, (int)t.p[2].y},
                t.shade
            );
        }
        VEC3D_TriangleVectorDestroy(list);
    }
    VEC3D_TriangleVectorDestroy(sortTri);
}
