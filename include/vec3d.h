#ifndef VEC3D_H
#define VEC3D_H
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>

#define COUNT_VEC3D_IN_T 3
#define VEC3D_TRIANGLE_SIZE 3
#define fNEAR 0.1f
#define fFAR 1000.0f
#define fFOV 90.0f
#define fFOV_RAD 1.0f / tanf(fFOV *0.5f / 180.f * 3.14159f)
#define VEC3D_DEFAULT ((Vec3d){0.0f, 0.0f, 0.0f, 1.0f})

typedef struct vec3d{
    float x,y,z,w;
} Vec3d;

static inline Vec3d VEC3D_Vec3dConstructor(float x, float y, float z){
    return (Vec3d){.w = 1.0f,.x = x,.y = y,.z = z};
}

typedef struct triangle {
    Vec3d p[COUNT_VEC3D_IN_T];
    uint8_t shade; 
} Triangle;



typedef struct triangleVector *TriangleVector; 

typedef struct meshCube* MeshCube;


//TriangleVector
TriangleVector   VEC3D_TriangleVectorCreate(void);
int              VEC3D_TriangleVectorPush(TriangleVector tv, Triangle t);
void             VEC3D_TriangleVectorDestroy(TriangleVector tv);

size_t           VEC3D_TriangleVectorSize  (TriangleVector tv);
const Triangle*  VEC3D_TriangleVectorData  (TriangleVector tv);
Triangle         VEC3D_TriangleVectorGetAt (TriangleVector tv, size_t index);
void             VEC3D_TriangleVectorSortByMidZ(TriangleVector tv);

//Vec3d
Vec3d            VEC3D_Vec3dAdd(Vec3d *v1, Vec3d *v2);
Vec3d            VEC3D_Vec3dSub(Vec3d *v1, Vec3d *v2);
Vec3d            VEC3D_Vec3dMul(Vec3d *v1, float k);
Vec3d            VEC3D_Vec3dDiv(Vec3d *v1, float k);
Vec3d            VEC3D_Vec3dCrossProduct(Vec3d *v1, Vec3d *v2);
float            VEC3D_Vec3dDotProduct(Vec3d *v1, Vec3d *v2);
void             VEC3D_Vec3dNormal(Vec3d* normal, const Triangle *trans);
Vec3d            VEC3D_Vec3dNormalize(Vec3d *v);
float            VEC3D_Vec3dLength(Vec3d *v);


//MeshCube
//MeshCube         VEC3D_CreateUnitCube(); 
void             VEC3D_DestroyCube(MeshCube c);
TriangleVector   VEC3D_GetCubeTriangles(MeshCube c);

#endif