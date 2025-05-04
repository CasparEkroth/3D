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
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            matProj->m[r][c] = 0.0f;

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

static void VEC3D_Matrix4x4Identity(Matrix4x4* mat) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            mat->m[r][c] = (r == c) ? 1.0f : 0.0f;
}

void VEC3D_Matrix4x4RotateZ(Matrix4x4* mat, float theta) {
    float c = cosf(theta);
    float s = sinf(theta);
    VEC3D_Matrix4x4Identity(mat);
    mat->m[0][0] =  c;  mat->m[0][1] = -s;
    mat->m[1][0] =  s;  mat->m[1][1] =  c;
    // z row/column and bottom row stay identity
}

void VEC3D_Matrix4x4RotateX(Matrix4x4* mat, float theta) {
    float c = cosf(theta);
    float s = sinf(theta);
    VEC3D_Matrix4x4Identity(mat);
    mat->m[1][1] =  c;  mat->m[1][2] = -s;
    mat->m[2][1] =  s;  mat->m[2][2] =  c;
    // x row/column and bottom row stay identity
}

void VEC3D_Vec3dNormal(Vec3d* normal, const Triangle *tri){
    Vec3d line1, line2;
    // edge from p0 → p1
    line1.x = tri->p[1].x - tri->p[0].x;
    line1.y = tri->p[1].y - tri->p[0].y;
    line1.z = tri->p[1].z - tri->p[0].z;

    line2.x = tri->p[2].x - tri->p[0].x;
    line2.y = tri->p[2].y - tri->p[0].y;
    line2.z = tri->p[2].z - tri->p[0].z;

    // cross product
    normal->x = line1.y * line2.z - line1.z * line2.y;
    normal->y = line1.z * line2.x - line1.x * line2.z;
    normal->z = line1.x * line2.y - line1.y * line2.x;

    VEC3D_Vec3dNormalize(normal);
}

void VEC3D_Vec3dNormalize(Vec3d *vec){
        float len = sqrtf(
        vec->x*vec->x +
        vec->y*vec->y +
        vec->z*vec->z
    );
    if (len > 1e-6f) {
        vec->x /= len;
        vec->y /= len;
        vec->z /= len;
    } else {
        // degenerate triangle: point vec straight out Z
        vec->x = vec->y = 0.0f;
        vec->z = 1.0f;
    }
}

static int compare_mid_z(const void *a, const void *b) {
    const Triangle *t1 = (const Triangle*)a;
    const Triangle *t2 = (const Triangle*)b;
    float z1 = (t1->p[0].z + t1->p[1].z + t1->p[2].z) / 3.0f;
    float z2 = (t2->p[0].z + t2->p[1].z + t2->p[2].z) / 3.0f;
    if (z1 < z2) return  1;   // z2 first
    if (z1 > z2) return -1;   // z1 first
    return 0;
}

void VEC3D_TriangleVectorSortByMidZ(TriangleVector tv){
    if (!tv || tv->size < 2) return;
    qsort(
        tv->data,
        tv->size,
        sizeof(tv->data[0]),
        compare_mid_z
    );
}