#include "../include/init.h"
#include "../include/draw3d.h"
#include "../include/loadFromObjectFile.h"

#define SPACE_SHIP "assets/Spaceship.obj"

typedef struct engine3D {
    TriangleVector mesh;   
    Matrix4x4       matProj;
    Vec3d           vCamera;
} Engine3D;



int main(int argc, char **argv){
    (void)argc;
    Game g = {0};
    SDL_Event event = {0};
    Engine3D e3D;

    if (!initialize_window(&g)) return 1;
    g.programIsRunning = true;

    //  — load from .obj if provided, else use unit cube —
    e3D.mesh = loadFromObjectFile(SPACE_SHIP);
    if (e3D.mesh == NULL) {
        fprintf(stderr, "Failed to load OBJ '%s'\n", argv[1]);
        // convert unit cube to a TriangleVector
        MeshCube cube = VEC3D_CreateUnitCube();
        e3D.mesh = VEC3D_TriangleVectorCreate();
        size_t n = VEC3D_TriangleVectorSize(VEC3D_GetCubeTriangles(cube));
        const Triangle *cTris = VEC3D_TriangleVectorData(VEC3D_GetCubeTriangles(cube));
        for (size_t i = 0; i < n; i++)
            VEC3D_TriangleVectorPush(e3D.mesh, cTris[i]);
        VEC3D_DestroyCube(cube);
    }

    // setup camera & projection
    e3D.vCamera = (Vec3d){0,0,0};
    VEC3D_Matrix4x4Proj(&e3D.matProj, g.pWindow);

    uint32_t startTicks = SDL_GetTicks();
    while (g.programIsRunning) {
        uint32_t now  = SDL_GetTicks();
        float    secs = (now - startTicks) / 3000.0f;  // seconds

        Matrix4x4 matRotZ, matRotX;
        VEC3D_Matrix4x4RotateZ(&matRotZ, secs * 2.0f * M_PI);
        VEC3D_Matrix4x4RotateX(&matRotX, secs *     M_PI);

        input(event, &g);
        update(&g);
        SDL_SetRenderDrawColor(g.pRend, 0, 0, 0, 255);
        SDL_RenderClear(g.pRend);
        DRAW3D_MeshRender(
            g.pRend,
            e3D.mesh,
            matRotZ, matRotX,
            e3D.matProj,
            e3D.vCamera
        );
        SDL_RenderPresent(g.pRend);
    }

    VEC3D_TriangleVectorDestroy(e3D.mesh);
    close_SDL(g.pWindow, g.pRend, &g);
    return 0;
}