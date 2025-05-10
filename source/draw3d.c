#include "../include/draw3d.h"

void DRAW3D_DrawTriangle(int x1,int y1, int x2,int y2, int x3,int y3, SDL_Renderer *pRend, uint8_t shade){
    SDL_SetRenderDrawColor(pRend, shade,shade,shade,255);
    SDL_RenderDrawLine(pRend, x1,y1, x2,y2);
    SDL_RenderDrawLine(pRend, x2,y2, x3,y3);
    SDL_RenderDrawLine(pRend, x3,y3, x1,y1);
    SDL_SetRenderDrawColor(pRend, 255,255,255,255);
}

void DRAW3D_DrawFillTriangle(SDL_Renderer* pRend, SDL_Point v0, SDL_Point v1, SDL_Point v2, uint8_t shade){
    SDL_SetRenderDrawColor(pRend,shade,shade,shade,255);
    // 1) Sort the vertices by Y ascending (v0.y <= v1.y <= v2.y)
    if (v1.y < v0.y) { SDL_Point t = v0; v0 = v1; v1 = t; }
    if (v2.y < v0.y) { SDL_Point t = v0; v0 = v2; v2 = t; }
    if (v2.y < v1.y) { SDL_Point t = v1; v1 = v2; v2 = t; }

    // 2) Compute inverse slopes for the long edge (v0→v2)
    float dx02 = (float)(v2.x - v0.x) / (float)(v2.y - v0.y);

    // 3) Rasterize upper half (v0→v1) and lower half (v1→v2)
    //    by stepping y and computing left/right X endpoints
    float xl = (float)v0.x;
    float xr = (float)v0.x;

    // Upper part: v0.y → v1.y
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
    // Lower part: v1.y → v2.y
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
    Matrix4x4        matProj,
    Vec3d            vCamera
) {
    int w,h;
    SDL_GetRendererOutputSize(pRend, &w, &h);

    size_t count = VEC3D_TriangleVectorSize(mesh);
    const Triangle *tris = VEC3D_TriangleVectorData(mesh);
    TriangleVector sortTri;
    sortTri = VEC3D_TriangleVectorCreate();
    for (size_t i = 0; i < count; i++) {
        // Fetch & transform the triangle
        Triangle in        = tris[i];
        Triangle  triProjected, triTransformd;
        // rotate
        triTransformd.p[0] = MATRIX_Matrix4x4MultiplyVector(&in.p[0], &matWorld);
        triTransformd.p[1] = MATRIX_Matrix4x4MultiplyVector(&in.p[1], &matWorld);
        triTransformd.p[2] = MATRIX_Matrix4x4MultiplyVector(&in.p[2], &matWorld);

        Vec3d normal,line1,line2;
        line1 = VEC3D_Vec3dSub(&triTransformd.p[1],&triTransformd.p[0]);
        line2 = VEC3D_Vec3dSub(&triTransformd.p[2],&triTransformd.p[0]);

        normal = VEC3D_Vec3dCrossProduct(&line1, &line2);

        normal = VEC3D_Vec3dNormalize(&normal);

        Vec3d vCameraRay = VEC3D_Vec3dSub(&triTransformd.p[0],&vCamera);

        if(VEC3D_Vec3dDotProduct(&normal,&vCameraRay) < 0.0f){
            Vec3d light_dir = {0.0f,0.0f,-1.0f,1.0f};
            light_dir = VEC3D_Vec3dNormalize(&light_dir);

            float dp_light = fmaxf(0.1f,VEC3D_Vec3dDotProduct(&normal,&light_dir));
            const float ambient = 0.1f;
            uint8_t shade = (uint8_t)((ambient + (1.0f-ambient)*dp_light)*255);
            triTransformd.shade = shade;
            // 3D --> 2D
            triProjected.p[0] = MATRIX_Matrix4x4MultiplyVector(&triTransformd.p[0],&matProj);
            triProjected.p[1] = MATRIX_Matrix4x4MultiplyVector(&triTransformd.p[1],&matProj);
            triProjected.p[2] = MATRIX_Matrix4x4MultiplyVector(&triTransformd.p[2],&matProj);
            triProjected.shade = triTransformd.shade;

            triProjected.p[0] = VEC3D_Vec3dDiv(&triProjected.p[0],triProjected.p[0].w);
            triProjected.p[1] = VEC3D_Vec3dDiv(&triProjected.p[1],triProjected.p[1].w);
            triProjected.p[2] = VEC3D_Vec3dDiv(&triProjected.p[2],triProjected.p[2].w);

            Vec3d vOffsetView = VEC3D_Vec3dConstructor(1.0f,1.0f,0.0f);
            triProjected.p[0] = VEC3D_Vec3dAdd(&triProjected.p[0],&vOffsetView);
            triProjected.p[1] = VEC3D_Vec3dAdd(&triProjected.p[1],&vOffsetView);
            triProjected.p[2] = VEC3D_Vec3dAdd(&triProjected.p[2],&vOffsetView);
            for (int i = 0; i < 3; i++){
                triProjected.p[i].x *= 0.5f * (float)w; 
                triProjected.p[i].y *= 0.5f * (float)h; 
            }
            
            Triangle screenTri = triProjected;           // copy the projected points
            screenTri.shade = triProjected.shade;      // bring over the lighting
            VEC3D_TriangleVectorPush(sortTri, screenTri);
        }
        
    }
    VEC3D_TriangleVectorSortByMidZ(sortTri);
    size_t sortedCount = VEC3D_TriangleVectorSize(sortTri);
    for (size_t i = 0; i < sortedCount; i++){
        // Draw filled triangle
        Triangle tres = VEC3D_TriangleVectorGetAt(sortTri,i);
        DRAW3D_DrawFillTriangle(
            pRend,
            (SDL_Point){ (int)tres.p[0].x, (int)tres.p[0].y },
            (SDL_Point){ (int)tres.p[1].x, (int)tres.p[1].y },
            (SDL_Point){ (int)tres.p[2].x, (int)tres.p[2].y },
            tres.shade
        );
        
    }
    VEC3D_TriangleVectorDestroy(sortTri);
}
