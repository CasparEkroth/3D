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
    Vec3d            vCamera) {
    int w, h;
    SDL_GetRendererOutputSize(pRend, &w, &h);

    size_t count = VEC3D_TriangleVectorSize(mesh);
    const Triangle *tris = VEC3D_TriangleVectorData(mesh);
    TriangleVector sortTri = VEC3D_TriangleVectorCreate();

    int nTotal=0, nVisible=0, nClipped=0, nProjected=0;

    for (size_t i = 0; i < count; i++) {
        nTotal++;
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
            nVisible++;

            // Lighting
            Vec3d light = VEC3D_Vec3dNormalize(&(Vec3d){0,0,-1,1});
            float dp = fmaxf(0.1f, VEC3D_Vec3dDotProduct(&normal, &light));
            uint8_t shade = (uint8_t)((0.1f + (1.0f-0.1f)*dp)*255);
            triTrans.shade = shade;

            // View transform
            triView.p[0] = MATRIX_Matrix4x4MultiplyVector(&triTrans.p[0], &matView);
            triView.p[1] = MATRIX_Matrix4x4MultiplyVector(&triTrans.p[1], &matView);
            triView.p[2] = MATRIX_Matrix4x4MultiplyVector(&triTrans.p[2], &matView);

            // Clip against near plane
            Triangle clipped[2];
            int c = VEC3D_ClipAgainstPlane((Vec3d){0,0,0.1f,1}, (Vec3d){0,0,1,1}, &triView, &clipped[0], &clipped[1]);
            nClipped += c;

            for (int j = 0; j < c; j++) {
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
                    proj.p[k].x = (proj.p[k].x+1.0f)*0.5f * w;
                    proj.p[k].y = (proj.p[k].y+1.0f)*0.5f * h;
                }

                VEC3D_TriangleVectorPush(sortTri, proj);
                nProjected++;
            }
        }
    }

    // Sort & draw
    VEC3D_TriangleVectorSortByMidZ(sortTri);
    for (size_t i = 0; i < VEC3D_TriangleVectorSize(sortTri); i++) {
        // 1) Start with one triangle in our list
        Triangle list[8];      // max grows: 1→2→4→8
        int   nList = 1;
        list[0] = VEC3D_TriangleVectorGetAt(sortTri, i);

        // 2) Define the four clipping planes in screen‐space
        //    (we ignore Z because we're clipping in 2D after projection)
        Vec3d plane_p[4] = {
            VEC3D_Vec3dConstructor(   0.0f,           0.0f, 0.0f),  // left
            VEC3D_Vec3dConstructor(   0.0f, (float)h - 1, 0.0f),  // bottom
            VEC3D_Vec3dConstructor(   0.0f,           0.0f, 0.0f),  // top
            VEC3D_Vec3dConstructor((float)w - 1,      0.0f, 0.0f)   // right
        };
        Vec3d plane_n[4] = {
            VEC3D_Vec3dConstructor( 1.0f,  0.0f, 0.0f),  // keep x >= 0
            VEC3D_Vec3dConstructor( 0.0f, -1.0f, 0.0f),  // keep y <= h-1
            VEC3D_Vec3dConstructor( 0.0f,  1.0f, 0.0f),  // keep y >= 0
            VEC3D_Vec3dConstructor(-1.0f,  0.0f, 0.0f)   // keep x <= w-1
        };

        // 3) Clip against each edge in turn
        for (int p = 0; p < 4; p++) {
            int nOut = 0;
            Triangle out[8]; // temporary buffer

            for (int tidx = 0; tidx < nList; tidx++) {
                Triangle clipped[2];
                int n = VEC3D_ClipAgainstPlane(
                    plane_p[p], plane_n[p],
                    &list[ tidx ],
                    &clipped[0],
                    &clipped[1]
                );
                // n==0 → fully outside, n==1 → untouched or partially, n==2 → split
                for (int k = 0; k < n; k++)
                    out[nOut++] = clipped[k];
            }

            // move back into list for the next edge pass
            nList = nOut;
            for (int k = 0; k < nList; k++)
                list[k] = out[k];

            // if nothing survives, skip remaining planes
            if (nList == 0) break;
        }

        // 4) Draw whatever’s left
        for (int tidx = 0; tidx < nList; tidx++) {
            Triangle *tt = &list[tidx];
            // convert Vec3d → SDL_Point
            SDL_Point v0 = { (int)tt->p[0].x, (int)tt->p[0].y };
            SDL_Point v1 = { (int)tt->p[1].x, (int)tt->p[1].y };
            SDL_Point v2 = { (int)tt->p[2].x, (int)tt->p[2].y };
            DRAW3D_DrawFillTriangle(pRend, v0, v1, v2, tt->shade);
        }
    }
    VEC3D_TriangleVectorDestroy(sortTri);
}
