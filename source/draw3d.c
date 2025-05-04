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

void DRAW3D_CubeRender(
    SDL_Renderer *pRend,
    MeshCube      cube,
    Matrix4x4     matRotZ,
    Matrix4x4     matRotX,
    Matrix4x4     matProj,
    Vec3d         vCamera
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

        Vec3d normal;
        VEC3D_Vec3dNormal(&normal,&trans);
        Vec3d cameraRay = {
            trans.p[0].x - vCamera.x,
            trans.p[0].y - vCamera.y,
            trans.p[0].z - vCamera.z 
        };
        float dp = normal.x * cameraRay.x + normal.y * cameraRay.y + normal.z * cameraRay.z;

        if(dp < 0.0f){
            //light
            Vec3d light_direction = {.x = 0.0f, .y = 0.0f, .z = -1.0f};
            VEC3D_Vec3dNormalize(&light_direction);
            float dp_light = normal.x * light_direction.x + normal.y * light_direction.y + normal.z * light_direction.z;
            const float ambient = 0.1f;
            if (dp_light < 0.0f) dp_light = 0.0f;
            trans.shade = (uint8_t)((ambient + (1.0f-ambient)*dp_light) * 255.0f);
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
            DRAW3D_DrawFillTriangle(pRend,
            (SDL_Point){.x = (int)proj.p[0].x, .y = (int)proj.p[0].y},
            (SDL_Point){.x = (int)proj.p[1].x, .y = (int)proj.p[1].y},
            (SDL_Point){.x = (int)proj.p[2].x, .y = (int)proj.p[2].y},
            trans.shade
            );
            // DRAW3D_DrawTriangle(
            //     (int)proj.p[0].x, (int)proj.p[0].y,
            //     (int)proj.p[1].x, (int)proj.p[1].y,
            //     (int)proj.p[2].x, (int)proj.p[2].y,
            //     pRend,
            //     trans.shede
            // );
        }
    }
}


void DRAW3D_MeshRender(
    SDL_Renderer*    pRend,
    TriangleVector  mesh,
    Matrix4x4        matRotZ,
    Matrix4x4        matRotX,
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
        Triangle in   = tris[i];
        Triangle rz, rxz, trans, proj;

        // Rotate Z
        VEC3D_Matrix4x4MultiplyVector(&in.p[0], &rz.p[0], &matRotZ);
        VEC3D_Matrix4x4MultiplyVector(&in.p[1], &rz.p[1], &matRotZ);
        VEC3D_Matrix4x4MultiplyVector(&in.p[2], &rz.p[2], &matRotZ);

        // Rotate X
        VEC3D_Matrix4x4MultiplyVector(&rz.p[0], &rxz.p[0], &matRotX);
        VEC3D_Matrix4x4MultiplyVector(&rz.p[1], &rxz.p[1], &matRotX);
        VEC3D_Matrix4x4MultiplyVector(&rz.p[2], &rxz.p[2], &matRotX);

        // Translate out of camera
        trans = rxz;
        for (int j = 0; j < VEC3D_TRIANGLE_SIZE; j++)
            trans.p[j].z += 8.0f;

        // Back‑face cull
        Vec3d normal;
        VEC3D_Vec3dNormal(&normal, &trans);

        Vec3d cameraRay = {
            trans.p[0].x - vCamera.x,
            trans.p[0].y - vCamera.y,
            trans.p[0].z - vCamera.z
        };
        float dp = normal.x*cameraRay.x
                 + normal.y*cameraRay.y
                 + normal.z*cameraRay.z;
        if (dp >= 0.0f) continue;

        // Lighting
        Vec3d light_dir = {0,0,-1};
        VEC3D_Vec3dNormalize(&light_dir);
        float dp_light = normal.x*light_dir.x + normal.y*light_dir.y + normal.z*light_dir.z;
        if (dp_light < 0.0f) dp_light = 0.0f;
        const float ambient = 0.1f;
        uint8_t shade = (uint8_t)((ambient + (1.0f-ambient)*dp_light)*255);
        trans.shade = shade;
        // Project
        for (int j = 0; j < VEC3D_TRIANGLE_SIZE; j++) {
            VEC3D_Matrix4x4MultiplyVector(&trans.p[j], &proj.p[j], &matProj);
            proj.p[j].x = (proj.p[j].x + 1.0f)*0.5f * w;
            proj.p[j].y = (proj.p[j].y + 1.0f)*0.5f * h;
        }

        
        Triangle screenTri = proj;           // copy the projected points
        screenTri.shade = trans.shade;      // bring over the lighting
        VEC3D_TriangleVectorPush(sortTri, screenTri);
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
