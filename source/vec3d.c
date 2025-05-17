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

// MeshCube VEC3D_CreateUnitCube(void) {
//     MeshCube c = malloc(sizeof *c);
//     if (!c) return NULL;

//     c->tris = VEC3D_TriangleVectorCreate();
//     if (!c->tris) {
//         free(c);
//         return NULL;
//     }

//     Vec3d v[8] = {
//         {0,0,0}, {1,0,0},
//         {1,1,0}, {0,1,0},
//         {0,0,1}, {1,0,1},
//         {1,1,1}, {0,1,1},
//     };

//     int idx[12][3] = {
//       {0,2,1},{0,3,2}, // -Z face
//       {4,5,6},{4,6,7}, // +Z
//       {0,4,7},{0,7,3}, // -X
//       {1,2,6},{1,6,5}, // +X
//       {0,1,5},{0,5,4}, // -Y
//       {3,7,6},{3,6,2}  // +Y
//     };

//     for (int i = 0; i < 12; i++) {
//         Triangle t;
//         t.p[0] = v[idx[i][0]];
//         t.p[1] = v[idx[i][1]];
//         t.p[2] = v[idx[i][2]];
//         if (VEC3D_TriangleVectorPush(c->tris, t) != 0) {
//             VEC3D_DestroyCube(c);
//             return NULL;
//         }
//     }

//     return c;
// }

// void VEC3D_DestroyCube(MeshCube c) {
//     if (!c) return;
//     VEC3D_TriangleVectorDestroy(c->tris);
//     free(c);
// }

// TriangleVector VEC3D_GetCubeTriangles(MeshCube c) {
//     return c ? c->tris : NULL;
// }

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

Vec3d VEC3D_Vec3dAdd(Vec3d *v1, Vec3d *v2){
    return (Vec3d){.x = (v1->x + v2->x),.y = (v1->y + v2->y),.z = (v1->z + v2->z)};
}

Vec3d VEC3D_Vec3dSub(Vec3d *v1, Vec3d *v2){
    return (Vec3d){.x = (v1->x - v2->x),.y = (v1->y - v2->y),.z = (v1->z - v2->z)};
}

Vec3d VEC3D_Vec3dMul(Vec3d *v1, float k){
    return (Vec3d){.x = (v1->x * k),.y = (v1->y * k),.z = (v1->z * k)};
}

Vec3d VEC3D_Vec3dDiv(Vec3d *v1, float k){
    return (Vec3d){.x = (v1->x / k),.y = (v1->y / k),.z = (v1->z / k)};
}

float VEC3D_Vec3dDotProduct(Vec3d *v1, Vec3d *v2){
    return (float)(v1->x * v2->x + v1->y * v2->y + v1->z * v2->z);
}

Vec3d VEC3D_Vec3dCrossProduct(Vec3d *v1, Vec3d *v2){
    Vec3d v;
    v.x = v1->y * v2->z - v1->z * v2->y;
    v.y = v1->z * v2->x - v1->x * v2->z;
    v.z = v1->x * v2->y - v1->y * v2->x;
    return v;
}

float VEC3D_Vec3dLength(Vec3d *v){
    return sqrtf(VEC3D_Vec3dDotProduct(v,v));
}

Vec3d VEC3D_Vec3dNormalize(Vec3d *v){
    float l = VEC3D_Vec3dLength(v);
    return (Vec3d){.x = v->x / l, .y = v->y / l, .z = v->z / l};
}

Vec3d VEC3D_Vec3dIntersectPlane(Vec3d *plane_p, Vec3d *plane_n, Vec3d *lineStart, Vec3d *lineEnd, float *t){
    *plane_n = VEC3D_Vec3dNormalize(plane_n);
    float plane_d = -VEC3D_Vec3dDotProduct(plane_n, plane_p);
    float ad = VEC3D_Vec3dDotProduct(lineStart, plane_n);
    float bd = VEC3D_Vec3dDotProduct(lineEnd, plane_n);
    (*t) = (-plane_d - ad) / (bd - ad);
    Vec3d lineStartToEnd = VEC3D_Vec3dSub(lineEnd, lineStart);
    float copy = *t;
    Vec3d lineToIntersect = VEC3D_Vec3dMul(&lineStartToEnd, copy);
    return VEC3D_Vec3dAdd(lineStart, &lineToIntersect);
}

static float VEC3D_DistanceToPlane(Vec3d plane_p, Vec3d plane_n, Vec3d p){
    return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z) - VEC3D_Vec3dDotProduct(&plane_n, &plane_p);
}

int VEC3D_ClipAgainstPlane(Vec3d plane_p, Vec3d plane_n, Triangle *in_tri, Triangle *out_tri1, Triangle *out_tri2){
    plane_n = VEC3D_Vec3dNormalize(&plane_n);
    float dist[3];
    for (int i = 0; i < 3; i++){
        dist[i]  = VEC3D_DistanceToPlane(plane_p, plane_n, in_tri->p[i]);
    }
    Vec3d* inside_points[3];    int nInsidePointCount = 0;
    Vec3d* outside_points[3];   int nOutsidePointCount = 0;
    Vec2d* inside_tex[3];       int nInsideTexCount = 0;
    Vec2d* outside_tex[3];      int nOutsideTexCount = 0; 

    for (int i = 0; i < 3; i++){
        if(dist[i] >= 0){ inside_points[nInsidePointCount++] = &in_tri->p[i]; inside_tex[nInsideTexCount++] = &in_tri->uv[i];}
        else { outside_points[nOutsidePointCount++] = &in_tri->p[i]; outside_tex[nOutsideTexCount++] = &in_tri->uv[i];}
    }
    if (nInsidePointCount == 0){
        return 0;
    }
    if(nInsidePointCount == 3){
        *out_tri1 = *in_tri;
        return 1;
    }
    if(nInsidePointCount == 1 && nOutsidePointCount == 2){
        out_tri1->shade = in_tri->shade;
        out_tri1->p[0] = *inside_points[0];
        out_tri1->uv[0] = *inside_tex[0];
        float t;
        out_tri1->p[1] = VEC3D_Vec3dIntersectPlane(&plane_p, &plane_n, inside_points[0],outside_points[0], &t);
        out_tri1->uv[1].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
        out_tri1->uv[1].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
        out_tri1->uv[1].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

        out_tri1->p[2] = VEC3D_Vec3dIntersectPlane(&plane_p, &plane_n, inside_points[0],outside_points[1], &t);
        out_tri1->uv[2].u = t * (outside_tex[1]->u - inside_tex[0]->u) + inside_tex[0]->u;
        out_tri1->uv[2].v = t * (outside_tex[1]->v - inside_tex[0]->v) + inside_tex[0]->v;
        out_tri1->uv[2].w = t * (outside_tex[1]->w - inside_tex[0]->w) + inside_tex[0]->w;
        return 1;
    }
    if(nInsidePointCount == 2 && nOutsidePointCount == 1){
        out_tri1->shade = in_tri->shade;
        out_tri2->shade = in_tri->shade;


        out_tri1->p[0] = *inside_points[0];
        out_tri1->p[1] = *inside_points[1];
        out_tri1->uv[0] = *inside_tex[0];
        out_tri1->uv[1] = *inside_tex[1];
        float t;
        out_tri1->p[2] = VEC3D_Vec3dIntersectPlane(&plane_p, &plane_n, inside_points[0], outside_points[0],&t);
        out_tri1->uv[2].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
        out_tri1->uv[2].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
        out_tri1->uv[2].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

        out_tri2->p[0] = *inside_points[1];
        out_tri2->uv[0] = *inside_tex[1];
        out_tri2->p[1] = out_tri1->p[2];
        out_tri2->uv[1] = out_tri1->uv[2];
        out_tri2->p[2] = VEC3D_Vec3dIntersectPlane(&plane_p, &plane_n, inside_points[1], outside_points[0], &t);
        out_tri2->uv[2].u = t * (outside_tex[0]->u - inside_tex[1]->u) + inside_tex[1]->u;
        out_tri2->uv[2].v = t * (outside_tex[0]->v - inside_tex[1]->v) + inside_tex[1]->v;
        out_tri2->uv[2].w = t * (outside_tex[0]->w - inside_tex[1]->w) + inside_tex[1]->w;
        return 2;
    }
    printf("out of bound\n");
}

Vec2d VEC2D_Vec2dConstructor(int u, int v){
    return(Vec2d){.v= v, .u = u, .w = 1.0f};
}