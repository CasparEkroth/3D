#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#include "../include/vec3d.h"
#include <math.h>

typedef struct Matrix4x4{
    float m[4][4];
}Matrix4x4;

//Matrix
Matrix4x4        MATRIX_Matrix4x4MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar);
Matrix4x4        MATRIX_Matrix4x4Identity(void);
Vec3d            MATRIX_Matrix4x4MultiplyVector(const Vec3d *v, const Matrix4x4 *m);
Matrix4x4        MATRIX_Matrix4x4RotateX(float fAngleRad);
Matrix4x4        MATRIX_Matrix4x4RotateY(float fAngleRad);
Matrix4x4        MATRIX_Matrix4x4RotateZ(float fAngleRad);
Matrix4x4        MATRIX_Matrix4x4MakeTranslatio(float x, float y, float z);
Matrix4x4        MATRIX_Matrix4x4MultiplyMatrix(Matrix4x4 *m1, Matrix4x4 *m2);



#endif
