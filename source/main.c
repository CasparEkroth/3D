#include "../include/init.h"
#include "../include/draw3d.h"
#include "../include/matrix.h"
#include "../include/loadFromObjectFile.h"

#define SPACE_SHIP "assets/Spaceship.obj"
#define TEPOT_PAHT "assets/tepot.obj"
#define AXIS_PAHT "assets/axis.obj"

typedef struct engine3D {
    TriangleVector mesh;   
    Matrix4x4      matProj;
    Vec3d          vCamera;
    Vec3d          vLookDir;
    float          fYaw;
    float          fPitch;
} Engine3D;

void update(Game *pGame,Engine3D *e3D);


int main(int argc, char **argv){
    (void)argc; (void)argv;
    Game g = {0};
    SDL_Event event = {0};
    Engine3D e3D = {0};

    if (!initialize_window(&g)) return 1;
    SDL_SetRelativeMouseMode(SDL_TRUE);
    g.programIsRunning = true;
    g.fElapsedTime = 0.0f;

    // — load mesh —
    e3D.mesh    = loadFromObjectFile(AXIS_PAHT);
    size_t nTris = VEC3D_TriangleVectorSize(e3D.mesh);
    printf("Loaded mesh with %zu triangles\n", nTris);
    if (nTris == 0) {
        fprintf(stderr, "Error: mesh has zero triangles!\n");
        return 1;
    }
    e3D.vCamera = (Vec3d){ 0.0f, 0.0f, 0.0f, 1.0f };  
    e3D.vLookDir = VEC3D_Vec3dConstructor(0,0,1.0f);
    e3D.fYaw = 0.0f;
    e3D.fPitch = 0.0f;
    int sw, sh;
    SDL_GetWindowSize(g.pWindow, &sw, &sh);
    e3D.matProj = MATRIX_Matrix4x4MakeProjection(90.0f, (float)sw/(float)sh, fNEAR, fFAR);

    uint32_t lastTick = SDL_GetTicks();
    while (g.programIsRunning) {
        uint32_t now = SDL_GetTicks();
        g.fElapsedTime = (now - lastTick) / 1000.0f;   // convert ms→s
        lastTick = now;
        Matrix4x4 matRotZ = MATRIX_Matrix4x4RotateZ(0 * 2.0f * M_PI);
        Matrix4x4 matRotX = MATRIX_Matrix4x4RotateX(0 * M_PI);

        Matrix4x4 matTrans = MATRIX_Matrix4x4MakeTranslatio(0.0f, 0.0f, 5.0f);

        Matrix4x4 matWorld = MATRIX_Matrix4x4Identity();
        matWorld = MATRIX_Matrix4x4MultiplyMatrix(&matRotZ, &matRotX);
        matWorld = MATRIX_Matrix4x4MultiplyMatrix(&matWorld, &matTrans);

        Vec3d vUp = VEC3D_Vec3dConstructor(0, 1.0f, 0);
        Vec3d vTarget = VEC3D_Vec3dConstructor(0, 0, 1.0f);
        Matrix4x4 matCameraRot = MATRIX_Matrix4x4RotateY(e3D.fYaw);
        e3D.vLookDir = MATRIX_Matrix4x4MultiplyVector(&vTarget, &matCameraRot);
        vTarget = VEC3D_Vec3dAdd(&e3D.vCamera, &e3D.vLookDir);

        Matrix4x4 matCamera = MATRIX_Matrix4x4PointAt(&e3D.vCamera, &vTarget, &vUp);

        Matrix4x4 matView = MATRIX_Matrix4x4QuickInverse(&matCamera);

        input(&event, &g);
        update(&g,&e3D);
        SDL_SetRenderDrawColor(g.pRend, 0, 0, 0, 255);
        SDL_RenderClear(g.pRend);

        DRAW3D_MeshRender(
            g.pRend,
            e3D.mesh,
            matWorld,    
            matView,
            e3D.matProj,     
            e3D.vCamera      
        );

        SDL_RenderPresent(g.pRend);
    }

    VEC3D_TriangleVectorDestroy(e3D.mesh);
    close_SDL(g.pWindow, g.pRend, &g);
    return 0;
}

void update(Game *pGame,Engine3D *e3D){
    int dx, dy;
    SDL_GetRelativeMouseState(&dx, &dy);

    Vec3d vForward = VEC3D_Vec3dMul(&e3D->vLookDir, 8.0f * pGame->fElapsedTime);

    const float rotSpeed = 1.0f;
    if (pGame->keys[SDL_SCANCODE_D]) e3D->fYaw -= rotSpeed * pGame->fElapsedTime;
    if (pGame->keys[SDL_SCANCODE_A]) e3D->fYaw += rotSpeed * pGame->fElapsedTime;

    if(pGame->keys[SDL_SCANCODE_W]) e3D->vCamera = VEC3D_Vec3dAdd(&e3D->vCamera,&vForward);
    if(pGame->keys[SDL_SCANCODE_S]) e3D->vCamera = VEC3D_Vec3dSub(&e3D->vCamera,&vForward);
    
    if(pGame->keys[SDL_SCANCODE_UP]) e3D->vCamera.y -= 8.0f * pGame->fElapsedTime;
    if(pGame->keys[SDL_SCANCODE_DOWN]) e3D->vCamera.y += 8.0f * pGame->fElapsedTime;
}