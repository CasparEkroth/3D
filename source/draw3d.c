#include "../include/draw3d.h"    
#include <stdio.h>                
#include <stdint.h>               
#include <stddef.h>               

#define SWAP(a,b) do { __typeof__(a) tmp = (a); (a) = (b); (b) = tmp; } while (0)

static float *gDepth = NULL;
static int    gW = 0, gH = 0;

static inline void put_pixel32(SDL_Surface *s, int x, int y, Uint32 c){
    Uint8 *p = (Uint8 *)s->pixels + y * s->pitch + x * 4;
    *(Uint32 *)p = c;
}

static inline Uint32 sample_texture(const SDL_Surface *tex, float u, float v){
    u = fminf(fmaxf(u,0.f),1.f);
    v = fminf(fmaxf(v,0.f),1.f);
    int tx = (int)((tex->w - 1) * u + 0.5f);
    int ty = (int)((tex->h - 1) * v + 0.5f);

    Uint8 *p = (Uint8*)tex->pixels + ty * tex->pitch + 
               tx * tex->format->BytesPerPixel;
    Uint32 pixel = 0;
    memcpy(&pixel, p, tex->format->BytesPerPixel);
    Uint8 r,g,b,a;
    SDL_GetRGBA(pixel, tex->format, &r,&g,&b,&a);
    return (a<<24)|(r<<16)|(g<<8)|b;
}


Uint32 get_pixel(SDL_Surface *surface, int x, int y) {
    if (!surface || x < 0 || x >= surface->w || y < 0 || y >= surface->h)
        return 0;

    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1: return *p;
        case 2: return *(Uint16 *)p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
        case 4: return *(Uint32 *)p;
        default: return 0;
    }
}



static inline void DrawPixel(SDL_Renderer *r, int x, int y, Uint32 colour){
    Uint8 a  = (colour >> 24) & 0xFF;
    Uint8 r8 = (colour >> 16) & 0xFF;
    Uint8 g8 = (colour >>  8) & 0xFF;
    Uint8 b8 =  colour        & 0xFF;

    SDL_SetRenderDrawBlendMode(r, a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE); 
    SDL_SetRenderDrawColor     (r, r8, g8, b8, a);
    SDL_RenderDrawPoint        (r, x, y);              
}

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
    Vec3d            vCamera,
    SDL_Texture*     tex
    ){
    int w, h;
    SDL_GetRendererOutputSize(pRend, &w, &h);

    if (w != gW || h != gH) {
        free(gDepth);                                
        gDepth = malloc(w * h * sizeof *gDepth);     
        gW = w;                                       
        gH = h;
    }
    memset(gDepth, 0, w * h * sizeof *gDepth);

    size_t count = VEC3D_TriangleVectorSize(mesh);
    const Triangle *tris = VEC3D_TriangleVectorData(mesh);
    TriangleVector sortTri = VEC3D_TriangleVectorCreate();
    for (size_t i = 0; i < count; i++) {
        Triangle in = tris[i];
        Triangle triTrans, triView, triProjected;

        // World transform
        for (int c = 0; c < 3; c++){
            triTrans.p[c] = MATRIX_Matrix4x4MultiplyVector(&in.p[c], &matWorld);
            triTrans.uv[c] = in.uv[c];
        } triTrans.shade = in.shade;
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
            for (int c = 0; c < 3; c++){
                triView.p[c] = MATRIX_Matrix4x4MultiplyVector(&triTrans.p[c], &matView);
                triView.uv[c] = triTrans.uv[c];
            } triView.shade = triTrans.shade;
            // Clip against near plane
            Triangle clipped[2];
            int nClipped = 0;
            nClipped = VEC3D_ClipAgainstPlane((Vec3d){0,0,0.1f,1}, (Vec3d){0,0,1.0f,1.0f}, &triView, &clipped[0], &clipped[1]);
            for (int j = 0; j < nClipped; j++) {
                // Project
                for (int c = 0; c < 3; c++){
                    triProjected.p[c] = MATRIX_Matrix4x4MultiplyVector(&clipped[j].p[c], &matProj);
                    triProjected.uv[c] = triView.uv[c];
                } triView.shade = triView.shade;

                if (triProjected.p[0].w<=0||triProjected.p[1].w<=0||triProjected.p[2].w<=0) continue;

                for (int c = 0; c < 3; c++){
                    triProjected.p[c] = VEC3D_Vec3dDiv(&triProjected.p[c],triProjected.p[c].w); 
                    triProjected.p[c].w=1;
                }
                triProjected.shade = clipped[j].shade;
                // Screen coords
                for (int k=0;k<3;k++) {
                    triProjected.p[k].x *= -1.0f; 
                    triProjected.p[k].y *= -1.0f;
                }
                Vec3d vOffsetView = VEC3D_Vec3dConstructor(1,1,0);
                for (int c = 0; c < 3; c++){
                    triProjected.p[c] = VEC3D_Vec3dAdd(&triProjected.p[c], &vOffsetView);
                    triProjected.p[c].x *= 0.5f * (float)w;
                    triProjected.p[c].y *= 0.5f * (float)h;
                }
                VEC3D_TriangleVectorPush(sortTri, triProjected);                
            }
        }
    }

    // Sort & draw
    VEC3D_TriangleVectorSortByMidZ(sortTri);
    float *pDepthBuffer = gDepth;
    for (size_t ti = 0; ti < VEC3D_TriangleVectorSize(sortTri); ti++) {
        Triangle t0 = VEC3D_TriangleVectorGetAt(sortTri, ti);
        TriangleVector list = VEC3D_TriangleVectorCreate();
        VEC3D_TriangleVectorPush(list, t0);

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

        for (size_t j = 0; j < VEC3D_TriangleVectorSize(list); j++) {
            Triangle t = VEC3D_TriangleVectorGetAt(list, j);
            // DRAW3D_DrawFillTriangle(
            //     pRend,
            //     (SDL_Point){(int)t.p[0].x, (int)t.p[0].y},
            //     (SDL_Point){(int)t.p[1].x, (int)t.p[1].y},
            //     (SDL_Point){(int)t.p[2].x, (int)t.p[2].y},
            //     t.shade);

            // TexturedTriangle(
            //     t.p[0].x, t.p[0].y, t.uv[0].u,t.uv[0].v, t.uv[0].w,
            //     t.p[1].x, t.p[1].y, t.uv[1].u,t.uv[1].v, t.uv[1].w,
            //     t.p[2].x, t.p[2].y, t.uv[2].u,t.uv[2].v, t.uv[2].w,
            //     texSurf, pRend, pDepthBuffer);

            TexturedTriangle_GPU(
                t.p[0].x, t.p[0].y, t.uv[0].u,t.uv[0].v,
                t.p[1].x, t.p[1].y, t.uv[1].u,t.uv[1].v,
                t.p[2].x, t.p[2].y, t.uv[2].u,t.uv[2].v,
                tex, pRend, t.shade);
        }
        VEC3D_TriangleVectorDestroy(list);
    }
    VEC3D_TriangleVectorDestroy(sortTri);
}


void TexturedTriangle(	int x1, int y1, float u1, float v1, float w1,
                        int x2, int y2, float u2, float v2, float w2,
                        int x3, int y3, float u3, float v3, float w3,
                        SDL_Surface *texSurf, SDL_Renderer *pRend, float *pDepthBuffer){
    int w, h;
    SDL_GetRendererOutputSize(pRend, &w, &h);
    if (y2 < y1){
        SWAP(y1, y2); SWAP(x1, x2); SWAP(u1, u2); SWAP(v1, v2); SWAP(w1, w2);
    }
    if (y3 < y1){
        SWAP(y1, y3); SWAP(x1, x3); SWAP(u1, u3); SWAP(v1, v3); SWAP(w1, w3);
    }
    if (y3 < y2){
        SWAP(y2, y3); SWAP(x2, x3); SWAP(u2, u3); SWAP(v2, v3); SWAP(w2, w3);
    }
    int dy1 = y2 - y1;
    int dx1 = x2 - x1;
    float dv1 = v2 - v1;
    float du1 = u2 - u1;
    float dw1 = w2 - w1;

    int dy2 = y3 - y1;
    int dx2 = x3 - x1;
    float dv2 = v3 - v1;
    float du2 = u3 - u1;
    float dw2 = w3 - w1;

    float tex_u, tex_v, tex_w;

    float dax_step = 0, dbx_step = 0,
        du1_step = 0, dv1_step = 0,
        du2_step = 0, dv2_step = 0,
        dw1_step=0, dw2_step=0;

    if (dy1) dax_step = dx1 / (float)abs(dy1);
    if (dy2) dbx_step = dx2 / (float)abs(dy2);

    if (dy1) du1_step = du1 / (float)abs(dy1);
    if (dy1) dv1_step = dv1 / (float)abs(dy1);
    if (dy1) dw1_step = dw1 / (float)abs(dy1);

    if (dy2) du2_step = du2 / (float)abs(dy2);
    if (dy2) dv2_step = dv2 / (float)abs(dy2);
    if (dy2) dw2_step = dw2 / (float)abs(dy2);

    if (dy1){
        for (int i = y1; i <= y2; i++){
            int ax = x1 + (float)(i - y1) * dax_step;
            int bx = x1 + (float)(i - y1) * dbx_step;

            float tex_su = u1 + (float)(i - y1) * du1_step;
            float tex_sv = v1 + (float)(i - y1) * dv1_step;
            float tex_sw = w1 + (float)(i - y1) * dw1_step;

            float tex_eu = u1 + (float)(i - y1) * du2_step;
            float tex_ev = v1 + (float)(i - y1) * dv2_step;
            float tex_ew = w1 + (float)(i - y1) * dw2_step;

            if (ax > bx){
                SWAP(ax, bx); SWAP(tex_su, tex_eu); SWAP(tex_sv, tex_ev); SWAP(tex_sw, tex_ew);
            }
            tex_u = tex_su;
            tex_v = tex_sv;
            tex_w = tex_sw;

            float tstep = 1.0f / ((float)(bx - ax));
            float t = 0.0f;

            for (int j = ax; j < bx; j++){
                tex_u = (1.0f - t) * tex_su + t * tex_eu;
                tex_v = (1.0f - t) * tex_sv + t * tex_ev;
                tex_w = (1.0f - t) * tex_sw + t * tex_ew;
                if (tex_w > pDepthBuffer[i * w + j]){
                    Uint32 colour = sample_texture(texSurf, tex_u/tex_w, tex_v/tex_w);
                    DrawPixel(pRend, j, i, colour);
                    pDepthBuffer[i * w + j] = tex_w;
                }
                t += tstep;
            }
        }
    }

    dy1 = y3 - y2;
    dx1 = x3 - x2;
    dv1 = v3 - v2;
    du1 = u3 - u2;
    dw1 = w3 - w2;

    if (dy1) dax_step = dx1 / (float)abs(dy1);
    if (dy2) dbx_step = dx2 / (float)abs(dy2);

    du1_step = 0, dv1_step = 0;
    if (dy1) du1_step = du1 / (float)abs(dy1);
    if (dy1) dv1_step = dv1 / (float)abs(dy1);
    if (dy1) dw1_step = dw1 / (float)abs(dy1);

    if (dy1){
        for (int i = y2; i <= y3; i++){
            int ax = x2 + (float)(i - y2) * dax_step;
            int bx = x1 + (float)(i - y1) * dbx_step;

            float tex_su = u2 + (float)(i - y2) * du1_step;
            float tex_sv = v2 + (float)(i - y2) * dv1_step;
            float tex_sw = w2 + (float)(i - y2) * dw1_step;

            float tex_eu = u1 + (float)(i - y1) * du2_step;
            float tex_ev = v1 + (float)(i - y1) * dv2_step;
            float tex_ew = w1 + (float)(i - y1) * dw2_step;

            if (ax > bx){
                SWAP(ax, bx); SWAP(tex_su, tex_eu); SWAP(tex_sv, tex_ev); SWAP(tex_sw, tex_ew);
            }

            tex_u = tex_su;
            tex_v = tex_sv;
            tex_w = tex_sw;

            float tstep = 1.0f / ((float)(bx - ax));
            float t = 0.0f;

            for (int j = ax; j < bx; j++){
                tex_u = (1.0f - t) * tex_su + t * tex_eu;
                tex_v = (1.0f - t) * tex_sv + t * tex_ev;
                tex_w = (1.0f - t) * tex_sw + t * tex_ew;
                if (tex_w > pDepthBuffer[i * w + j]){
                    Uint32 pixel = get_pixel(texSurf, tex_u, tex_v);
                    Uint8 r, g, b, a;
                    SDL_GetRGBA(pixel, texSurf->format, &r, &g, &b, &a);
                    if (a == 0) continue;
                    SDL_SetRenderDrawColor(pRend, r, g, b, a);
                    SDL_RenderDrawPoint(pRend, j, i);
                    
                    pDepthBuffer[i * w + j] = tex_w;
                }
                t += tstep;
            }
        }	
    }		
}

void DRAW3D_Shutdown(void){
    free(gDepth);
    gDepth = NULL;
    gW = gH = 0;
}

void TexturedTriangle_GPU(
    int x1, int y1, float u1, float v1,
    int x2, int y2, float u2, float v2,
    int x3, int y3, float u3, float v3,
    SDL_Texture *tex, SDL_Renderer *r,
    uint8_t shade){
    const float UV_SCALE = 1.0f / 65535.0f;

    SDL_Vertex verts[3] = {
        {.position = { (float)x1, (float)y1 },
        .color    = { shade, shade, shade, 255 },
        .tex_coord = { u1 * UV_SCALE, v1 * UV_SCALE } },

        {.position = { (float)x2, (float)y2 },
        .color    = { shade, shade, shade, 255 },
        .tex_coord = { u2 * UV_SCALE, v2 * UV_SCALE } },

        {.position = { (float)x3, (float)y3 },
        .color    = { shade, shade, shade, 255 },
        .tex_coord = { u3 * UV_SCALE, v3 * UV_SCALE } }
    };

    int indices[3] = { 0, 1, 2 };
    SDL_RenderGeometry(r, tex, verts, 3, indices, 3);
}
