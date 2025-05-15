#include "../include/init.h"
#include "../include/draw3d.h"
#include "../include/matrix.h"
#include "../include/loadFromObjectFile.h"

#define SPACE_SHIP "assets/Spaceship.obj"
#define TEPOT_PAHT "assets/tepot.obj"
#define AXIS_PAHT "assets/axis.obj"

#define TEXTUR_OBJ "assets/texturdTest1/test_sphere.obj"
#define TEXTUR_PNG "assets/texturdTest1/checker_texture.png"
#define TEXTUR_NOISE "assets/texturdTest1/noise_texture.png"
typedef struct engine3D {
    TriangleVector mesh;   
    Matrix4x4      matProj;
    Vec3d          vCamera;
    Vec3d          vLookDir;
    Vec3d          vRight;
    Vec3d          vUp;
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
    SDL_ShowCursor(SDL_FALSE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    g.programIsRunning = true;
    g.fElapsedTime = 0.0f;

    // — load mesh —
    e3D.mesh    = loadFromObjectFile(TEXTUR_OBJ);
    SDL_Surface *orig = IMG_Load(TEXTUR_NOISE);
    SDL_Surface *texSurf = SDL_ConvertSurfaceFormat(orig,
    SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(orig);
    size_t nTris = VEC3D_TriangleVectorSize(e3D.mesh);
    printf("Loaded mesh with %zu triangles\n", nTris);
    if (nTris == 0) {
        fprintf(stderr, "Error: mesh has zero triangles!\n");
        return 1;
    }
    e3D.vCamera = VEC3D_DEFAULT;  
    e3D.vLookDir = VEC3D_Vec3dConstructor(0,0,1.0f);
    e3D.fYaw = 0.0f;
    e3D.fPitch = 0.0f;
    uint32_t lastTick = SDL_GetTicks();
    int sw, sh;
    SDL_GetWindowSize(g.pWindow, &sw, &sh);
    e3D.matProj = MATRIX_Matrix4x4MakeProjection(90.0f, (float)sw/(float)sh, fNEAR, fFAR);

    while (g.programIsRunning) {
        uint32_t now = SDL_GetTicks();
        g.fElapsedTime = (now - lastTick) / 1000.0f;   
        lastTick = now;

        input(&event, &g,&e3D.fYaw,&e3D.fPitch);
        update(&g,&e3D);

        // — Build world matrix for mesh —
        Matrix4x4 matRotZ = MATRIX_Matrix4x4RotateZ(0 * 2.0f * M_PI);
        Matrix4x4 matRotX = MATRIX_Matrix4x4RotateX(0 * M_PI);
        Matrix4x4 matTrans = MATRIX_Matrix4x4MakeTranslatio(0.0f, 0.0f, 5.0f);
        Matrix4x4 matWorld = MATRIX_Matrix4x4Identity();
        matWorld = MATRIX_Matrix4x4MultiplyMatrix(&matRotZ, &matRotX);
        matWorld = MATRIX_Matrix4x4MultiplyMatrix(&matWorld, &matTrans);

        // — Build view matrix from camera —
        Vec3d vTarget =  VEC3D_Vec3dAdd(&e3D.vCamera, &e3D.vLookDir);
        Matrix4x4 matCamera = MATRIX_Matrix4x4PointAt(&e3D.vCamera, &vTarget, &e3D.vUp);
        Matrix4x4 matView = MATRIX_Matrix4x4QuickInverse(&matCamera);

        SDL_SetRenderDrawColor(g.pRend, 0, 0, 0, 255);
        SDL_RenderClear(g.pRend);

        DRAW3D_MeshRender(
            g.pRend,
            e3D.mesh,
            matWorld,    
            matView,
            e3D.matProj,     
            e3D.vCamera,
            texSurf    
        );

        SDL_RenderPresent(g.pRend);
    }
    SDL_FreeSurface(texSurf);
    DRAW3D_Shutdown();
    VEC3D_TriangleVectorDestroy(e3D.mesh);
    close_SDL(g.pWindow, g.pRend, &g);
    return 0;
}

void update(Game *pGame,Engine3D *e3D){
    static const Vec3d vWorldUp = { 0.0f, 1.0f, 0.0f, 1.0f};
    Vec3d vForward = VEC3D_Vec3dConstructor(0,0,0);
    vForward.x = cosf(e3D->fPitch) * sinf(e3D->fYaw);
    vForward.y = sinf(e3D->fPitch);
    vForward.z = cosf(e3D->fPitch) * cosf(e3D->fYaw);
    VEC3D_Vec3dNormalize(&vForward);
    e3D->vLookDir = vForward;
    e3D->vRight = VEC3D_Vec3dCrossProduct(&e3D->vLookDir, &vWorldUp);
    VEC3D_Vec3dNormalize(&e3D->vRight);
    e3D->vUp = VEC3D_Vec3dCrossProduct(&e3D->vRight, &e3D->vLookDir);
    VEC3D_Vec3dNormalize(&e3D->vUp);

    const float fSpeed = 8.0f * pGame->fElapsedTime;
    Vec3d vMove  = VEC3D_Vec3dConstructor(0,0,0);
    Vec3d tmp = VEC3D_Vec3dConstructor(0,0,0);

    if (pGame->keys[SDL_SCANCODE_W]){
        tmp = VEC3D_Vec3dMul(&e3D->vLookDir,  fSpeed);
        vMove = VEC3D_Vec3dAdd(&vMove, &tmp);
    } 
    if (pGame->keys[SDL_SCANCODE_S]){
        tmp = VEC3D_Vec3dMul(&e3D->vLookDir, -fSpeed);
        vMove = VEC3D_Vec3dAdd(&vMove, &tmp);
    } 
    if (pGame->keys[SDL_SCANCODE_D]){
        tmp = VEC3D_Vec3dMul(&e3D->vRight, fSpeed);
        vMove = VEC3D_Vec3dAdd(&vMove, &tmp);
    } 
    if (pGame->keys[SDL_SCANCODE_A]){
        tmp = VEC3D_Vec3dMul(&e3D->vRight, -fSpeed);
        vMove = VEC3D_Vec3dAdd(&vMove, &tmp);
    } 
    if (pGame->keys[SDL_SCANCODE_LSHIFT]){
        vMove.y -= fSpeed;
    }
    if(pGame->keys[SDL_SCANCODE_SPACE]){
        vMove.y += fSpeed;
    }
    e3D->vCamera = VEC3D_Vec3dAdd(&e3D->vCamera, &vMove);
}