#include "../include/init.h"
#include "../include/draw3d.h"
#include "../include/matrix4x4.h"
#include "../include/loadFromObjectFile.h"

#define SPACE_SHIP "assets/Spaceship.obj"

typedef struct engine3D {
    TriangleVector mesh;   
    Matrix4x4      matProj;
    Vec3d          vCamera;
} Engine3D;

int main(int argc, char **argv){
    (void)argc; (void)argv;
    Game g = {0};
    SDL_Event event = {0};
    Engine3D e3D;

    if (!initialize_window(&g)) return 1;
    g.programIsRunning = true;

    // — load mesh —
    e3D.mesh    = loadFromObjectFile(SPACE_SHIP);
    e3D.vCamera = (Vec3d){ 0.0f, 0.0f, 0.0f, 1.0f };  

    int sw, sh;
    SDL_GetWindowSize(g.pWindow, &sw, &sh);
    e3D.matProj = MATRIX_Matrix4x4MakeProjection(fFOV, (float)sh/(float)sw, fNEAR, fFAR);

    uint32_t startTicks = SDL_GetTicks();
    while (g.programIsRunning) {
        // elapsed time drives rotation
        uint32_t now  = SDL_GetTicks();
        float    secs = (now - startTicks) / 3000.0f;

        // — build rotation matrices —
        Matrix4x4 matRotZ = MATRIX_Matrix4x4RotateZ(secs * 2.0f * M_PI);
        Matrix4x4 matRotX = MATRIX_Matrix4x4RotateX(secs *       M_PI);

        // — build translation 5 units into screen —
        Matrix4x4 matTrans = MATRIX_Matrix4x4MakeTranslatio(0.0f, 0.0f, 16.0f);

        // — combine into one World matrix —
        //   World = Identity → RotateZ → RotateX → Translate
        Matrix4x4 matWorld = MATRIX_Matrix4x4Identity();
        matWorld = MATRIX_Matrix4x4MultiplyMatrix(&matRotZ, &matRotX);
        matWorld = MATRIX_Matrix4x4MultiplyMatrix(&matWorld, &matTrans);

        // handle input & clear
        input(event, &g);
        update(&g);
        SDL_SetRenderDrawColor(g.pRend, 0, 0, 0, 255);
        SDL_RenderClear(g.pRend);

        // render with a single World matrix
        DRAW3D_MeshRender(
            g.pRend,
            e3D.mesh,
            matWorld,          // <- world transform
            e3D.matProj,       // <- projection
            e3D.vCamera        // <- camera pos (w=1!)
        );
        SDL_RenderPresent(g.pRend);
    }

    VEC3D_TriangleVectorDestroy(e3D.mesh);
    close_SDL(g.pWindow, g.pRend, &g);
    return 0;
}
