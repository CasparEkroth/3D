# include "../include/matrix.h"

Matrix4x4 MATRIX_Matrix4x4Identity(void){
    Matrix4x4 m = {0};
    m.m[0][0] = m.m[1][1] = m.m[2][2] = m.m[3][3] = 1.0f;
    return m;
}

Vec3d MATRIX_Matrix4x4MultiplyVector(const Vec3d *v, const Matrix4x4 *m){
    Vec3d vec;
    vec.x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0] + v->w * m->m[3][0];
    vec.y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1] + v->w * m->m[3][1];
    vec.z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2] + v->w * m->m[3][2];
    vec.w = v->x * m->m[0][3] + v->y * m->m[1][3] + v->z * m->m[2][3] + v->w * m->m[3][3];
    return vec;
}

Matrix4x4 MATRIX_Matrix4x4RotateY(float fAngleRad){
    Matrix4x4 mat = MATRIX_Matrix4x4Identity();
    mat.m[0][0] = cosf(fAngleRad);
    mat.m[0][2] = sinf(fAngleRad);
    mat.m[2][0] = -sinf(fAngleRad);
    mat.m[1][1] = 1.0f;
    mat.m[2][2] = cosf(fAngleRad);
    mat.m[3][3] = 1.0f;
    return mat;
}

Matrix4x4 MATRIX_Matrix4x4RotateX(float fAngleRad){
    Matrix4x4 m = MATRIX_Matrix4x4Identity();
    m.m[1][1] =  cosf(fAngleRad);
    m.m[1][2] = -sinf(fAngleRad);
    m.m[2][1] =  sinf(fAngleRad);
    m.m[2][2] =  cosf(fAngleRad);
    return m;
}

Matrix4x4 MATRIX_Matrix4x4RotateZ(float fAngleRad){
    Matrix4x4 m = MATRIX_Matrix4x4Identity();
    m.m[0][0] =  cosf(fAngleRad);
    m.m[0][1] = -sinf(fAngleRad);
    m.m[1][0] =  sinf(fAngleRad);
    m.m[1][1] =  cosf(fAngleRad);
    return m;
}


Matrix4x4 MATRIX_Matrix4x4MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar){
    float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
    Matrix4x4 mat = {0};
    mat.m[0][0] = fAspectRatio * fFovRad;
    mat.m[1][1] = fFovRad;
    mat.m[2][2] = fFar / (fFar -fNear);
    mat.m[3][2] = (-fFar * fNear) / (fFar -fNear);
    mat.m[2][3] = 1.0f;
    mat.m[3][3] = 0.0f;
    return mat;
}

Matrix4x4 MATRIX_Matrix4x4MakeTranslatio(float x, float y, float z){
    Matrix4x4 mat = MATRIX_Matrix4x4Identity();
    mat.m[3][0] = x;
    mat.m[3][1] = y;
    mat.m[3][2] = z;
    return mat;
}

Matrix4x4 MATRIX_Matrix4x4MultiplyMatrix(Matrix4x4 *m1, Matrix4x4 *m2){
    Matrix4x4 mat = {0};
    for (int c = 0; c < 4; c++){
        for (int r = 0; r < 4; r++){
            mat.m[r][c] = m1->m[r][0] * m2->m[0][c] + m1->m[r][1] * m2->m[1][c] + m1->m[r][2] * m2->m[2][c] + m1->m[r][3] * m2->m[3][c];
        }
    }
    return mat;
}

Matrix4x4 MATRIX_Matrix4x4PointAt(Vec3d *pos,Vec3d *target, Vec3d *up){
    Vec3d newForward = VEC3D_Vec3dSub(target, pos);
    newForward = VEC3D_Vec3dNormalize(&newForward);

    Vec3d a = VEC3D_Vec3dMul(&newForward, VEC3D_Vec3dDotProduct(up, &newForward));
    Vec3d newUp = VEC3D_Vec3dSub(up, &a);
    newUp = VEC3D_Vec3dNormalize(&newUp);

    Vec3d newRight = VEC3D_Vec3dCrossProduct(&newUp, &newForward);

    Matrix4x4 mat = {0};
    mat.m[0][0] = newRight.x;   mat.m[0][1] = newRight.y;   mat.m[0][2] = newRight.z;   mat.m[0][3] = 0.0f; 
    mat.m[1][0] = newUp.x;      mat.m[1][1] = newUp.y;      mat.m[1][2] = newUp.z;      mat.m[1][3] = 0.0f; 
    mat.m[2][0] = newForward.x; mat.m[2][1] = newForward.y; mat.m[2][2] = newForward.z; mat.m[2][3] = 0.0f; 
    mat.m[3][0] = pos->x;       mat.m[3][1] = pos->y;       mat.m[3][2] = pos->z;       mat.m[3][3] = 0.0f; 
    return mat;
}

Matrix4x4 MATRIX_Matrix4x4QuickInverse(Matrix4x4 *m){
    Matrix4x4 mat = {0};
    mat.m[0][0] = m->m[0][0]; mat.m[0][1] = m->m[1][0]; mat.m[0][2] = m->m[2][0]; mat.m[0][3] = 0.0f;
    mat.m[1][0] = m->m[0][1]; mat.m[1][1] = m->m[1][1]; mat.m[1][2] = m->m[2][1]; mat.m[1][3] = 0.0f;
    mat.m[2][0] = m->m[0][2]; mat.m[2][1] = m->m[1][2]; mat.m[2][2] = m->m[2][2]; mat.m[2][3] = 0.0f;

    mat.m[3][0] = -(m->m[3][0] * mat.m[0][0] + m->m[3][1] * mat.m[1][0] + m->m[3][2] * mat.m[2][0]);
    mat.m[3][1] = -(m->m[3][0] * mat.m[0][1] + m->m[3][1] * mat.m[1][1] + m->m[3][2] * mat.m[2][1]);
    mat.m[3][2] = -(m->m[3][0] * mat.m[0][2] + m->m[3][1] * mat.m[1][2] + m->m[3][2] * mat.m[2][2]);
    mat.m[3][3] = 1.0f;
    return mat;
}