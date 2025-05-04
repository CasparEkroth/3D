#ifndef VEC3D_H
#define VEC3D_H
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>

#define COUNT_VEC3D_IN_T 3
#define fNEAR 0.1f
#define fFAR 1000.0f
#define fFOV 90.0f
#define fFOV_RAD 1.0f / tanf(fFOV *0.5f / 180.f * 3.14159f)

typedef struct vec3d{
    float x,y,z;
} Vec3d;


typedef struct triangle {
    Vec3d p[COUNT_VEC3D_IN_T];
} Triangle;

typedef struct matrix4x4{
    float m[4][4];
}Matrix4x4;

typedef struct triangleVector *TriangleVector; 

typedef struct meshCube* MeshCube;


//TriangleVector
TriangleVector   VEC3D_TriangleVectorCreate(void);
int              VEC3D_TriangleVectorPush(TriangleVector tv, Triangle t);
void             VEC3D_TriangleVectorDestroy(TriangleVector tv);

size_t           VEC3D_TriangleVectorSize  (TriangleVector tv);
const Triangle*  VEC3D_TriangleVectorData  (TriangleVector tv);
Triangle         VEC3D_TriangleVectorGetAt (TriangleVector tv, size_t index);

//MeshCube
MeshCube         VEC3D_CreateUnitCube(); 
void             VEC3D_DestroyCube(MeshCube c);
TriangleVector   VEC3D_GetCubeTriangles(MeshCube c);

//Matrix
void VEC3D_Matrix4x4Proj(Matrix4x4 * matProj,SDL_Window *pWin);
void VEC3D_Matrix4x4MultiplyVector(const Vec3d *in, Vec3d *out, const Matrix4x4 *m);


#endif