#include "../include/init.h"
#include "../include/draw3d.h"

#define NAME 40

int main(int argc, char **argv){
    (void)argc;
    (void)argv;
    Game g = {0};
    SDL_Event event = {0};
    
    if(initialize_window(&g)){
    g.programIsRunning = true;
    }else g.programIsRunning = false;
    MeshCube cube = VEC3D_CreateUnitCube();
    if (!cube) {
        fprintf(stderr, "Failed to create cube mesh\n");
        return 1;
    }

    Matrix4x4 matProj;
    VEC3D_Matrix4x4Proj(&matProj,g.pWindow);
    uint32_t startTicks = SDL_GetTicks();

    while (g.programIsRunning){
        uint32_t now   = SDL_GetTicks();
        float    secs  = (now - startTicks) / 3000.0f;
        Matrix4x4 matRotZ, matRotX;
        VEC3D_Matrix4x4RotateZ(&matRotZ, secs * 2.0f * M_PI);
        VEC3D_Matrix4x4RotateX(&matRotX, secs *     M_PI);
        input(event,&g);
        update(&g);
        SDL_RenderClear(g.pRenderere);
        DRAW3D_CubeRender(g.pRenderere, cube, matRotZ, matRotX, matProj);
        SDL_SetRenderDrawColor(g.pRenderere,0,0,0,0);
        SDL_RenderPresent(g.pRenderere);
    }
    VEC3D_DestroyCube(cube);
    close_SDL(g.pWindow,g.pRenderere,&g);
    return 0;
}