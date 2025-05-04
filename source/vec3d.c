// vec3d.c
#include "../include/vec3d.h"

struct triangleVector {
    Triangle *data;
    size_t    size;
    size_t    capacity;
};

struct meshCube {
    TriangleVector tris; 
};

TriangleVector VEC3D_TriangleVectorCreate(void){
    TriangleVector tv = malloc(sizeof *tv);
    if (!tv) return NULL;
    tv->data     = NULL;
    tv->size     = 0;
    tv->capacity = 0;
    return tv;
}

int VEC3D_TriangleVectorPush(TriangleVector tv, Triangle t){
    if (tv->size == tv->capacity) {
        size_t new_cap = tv->capacity ? tv->capacity * 2 : 4;
        Triangle *tmp = realloc(tv->data, new_cap * sizeof *tmp);
        if (!tmp) return -1;
        tv->data     = tmp;
        tv->capacity = new_cap;
    }
    tv->data[tv->size++] = t;
    return 0;
}

void VEC3D_TriangleVectorDestroy(TriangleVector tv){
    if (!tv) return;
    free(tv->data);
    free(tv);
}

size_t VEC3D_TriangleVectorSize(TriangleVector tv) {
    return tv ? tv->size : 0;
}

const Triangle* VEC3D_TriangleVectorData(TriangleVector tv) {
    return tv ? tv->data : NULL;
}

Triangle VEC3D_TriangleVectorGetAt(TriangleVector tv, size_t index) {
    return tv->data[index];
}

MeshCube VEC3D_CreateUnitCube(void) {
    MeshCube c = malloc(sizeof *c);
    if (!c) return NULL;

    c->tris = VEC3D_TriangleVectorCreate();
    if (!c->tris) {
        free(c);
        return NULL;
    }

    Vec3d v[8] = {
        {0,0,0}, {1,0,0},
        {1,1,0}, {0,1,0},
        {0,0,1}, {1,0,1},
        {1,1,1}, {0,1,1},
    };

    int idx[12][3] = {
      {0,2,1},{0,3,2}, // -Z face
      {4,5,6},{4,6,7}, // +Z
      {0,4,7},{0,7,3}, // -X
      {1,2,6},{1,6,5}, // +X
      {0,1,5},{0,5,4}, // -Y
      {3,7,6},{3,6,2}  // +Y
    };

    for (int i = 0; i < 12; i++) {
        Triangle t;
        t.p[0] = v[idx[i][0]];
        t.p[1] = v[idx[i][1]];
        t.p[2] = v[idx[i][2]];
        if (VEC3D_TriangleVectorPush(c->tris, t) != 0) {
            VEC3D_DestroyCube(c);
            return NULL;
        }
    }

    return c;
}

void VEC3D_DestroyCube(MeshCube c) {
    if (!c) return;
    VEC3D_TriangleVectorDestroy(c->tris);
    free(c);
}

TriangleVector VEC3D_GetCubeTriangles(MeshCube c) {
    return c ? c->tris : NULL;
}

void VEC3D_Matrix4x4Proj(Matrix4x4 * matProj,SDL_Window *pWin){
    int sw = 0,sh = 0;
    SDL_GetWindowSize(pWin, &sw, &sh);
    float fAspectRatio = (float)sh / (float)sw;
    matProj->m[0][0] = fAspectRatio * fFOV_RAD;
    matProj->m[1][1] = fFOV_RAD;
    matProj->m[2][2] = fFAR / (fFAR -fNEAR);
    matProj->m[3][2] = (-fFAR * fNEAR) / (fFAR -fNEAR);
    matProj->m[2][3] = 1.0f;
    matProj->m[3][3] = 1.0f;
}

void VEC3D_Matrix4x4MultiplyVector(const Vec3d *in, Vec3d *out, const Matrix4x4 *m){
    float x = in->x * m->m[0][0] + in->y * m->m[1][0] + in->z * m->m[2][0] + 1.0f * m->m[3][0];
    float y = in->x * m->m[0][1] + in->y * m->m[1][1] + in->z * m->m[2][1] + 1.0f * m->m[3][1];
    float z = in->x * m->m[0][2] + in->y * m->m[1][2] + in->z * m->m[2][2] + 1.0f * m->m[3][2];
    float w = in->x * m->m[0][3] + in->y * m->m[1][3] + in->z * m->m[2][3] + 1.0f * m->m[3][3];
    if (w != 0.0f) {
        out->x = x / w;
        out->y = y / w;
        out->z = z / w;
    } else {
        out->x = x;
        out->y = y;
        out->z = z;
    }
}
