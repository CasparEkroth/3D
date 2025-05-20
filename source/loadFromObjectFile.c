#include <ctype.h>    
#include <stdlib.h>  
#include <string.h>  
#include "../include/loadFromObjectFile.h"

typedef struct { int v, vt; } FaceIdx;

TriangleVector loadFromObjectFile(const char* fileName) {
    FILE *fp = fopen(fileName, "r");
    if (!fp) return NULL;

    Vec3d *verts3d = NULL;  size_t v3Count = 0, v3Cap = 0;
    Vec2d *verts2d = NULL;  size_t v2Count = 0, v2Cap = 0;
    TriangleVector tv = VEC3D_TriangleVectorCreate();
    if (!tv) { fclose(fp); return NULL; }

    char line[512];
    bool error = false;
    while (!error && fgets(line, sizeof line, fp)) {
        char *p = line;
        while (*p==' '||*p=='\t') ++p;
        if (*p=='v' && isspace((unsigned char)p[1])) {
            float x,y,z;
            if (sscanf(p+1, "%f %f %f", &x,&y,&z)==3) {
                if (v3Count==v3Cap) {
                    size_t newC = v3Cap ? v3Cap*2 : 8;
                    Vec3d *tmp = realloc(verts3d, newC*sizeof *tmp);
                    if (!tmp) { error=true; break; }
                    verts3d = tmp; v3Cap = newC;
                }
                verts3d[v3Count++] = VEC3D_Vec3dConstructor(x,y,z);
            }
        }
        else if (*p=='v' && p[1]=='t' && isspace((unsigned char)p[2])) {
            float u,v;
            if (sscanf(p+2, "%f %f", &u,&v)==2) {
                if (v2Count==v2Cap) {
                    size_t newC = v2Cap ? v2Cap*2 : 8;
                    Vec2d *tmp = realloc(verts2d, newC*sizeof *tmp);
                    if (!tmp) { error=true; break; }
                    verts2d = tmp; v2Cap = newC;
                }
                verts2d[v2Count++] = VEC2D_Vec2dConstructor((int)(u*65535), (int)(v*65535));
            }
        }
        else if (*p=='f' && isspace((unsigned char)p[1])) {
            FaceIdx idx[64];
            int count = 0;
            for (char *tok = strtok(p+1, " \t\r\n");
                 tok && count < (int)(sizeof idx/sizeof *idx);
                 tok = strtok(NULL, " \t\r\n"))
            {
                int v=0, vt=0;
                char *s = tok;
                v  = (int)strtol(s, &s, 10);
                if (*s=='/') {
                    ++s;
                    vt = (int)strtol(s, &s, 10);
                }
                idx[count++] = (FaceIdx){ v, vt };
            }
            for (int i = 1; i+1 < count; ++i) {
                Triangle t = { .shade = 255 };
                FaceIdx f0 = idx[0], f1 = idx[i], f2 = idx[i+1];
                FaceIdx tri[3] = { f0, f1, f2 };
                bool bad = false;
                for (int k = 0; k < 3; ++k) {
                    int vi  = tri[k].v  < 0 ? (int)v3Count + tri[k].v  : tri[k].v  - 1;
                    int vti = tri[k].vt < 0 ? (int)v2Count + tri[k].vt : tri[k].vt - 1;
                    if (vi<0 || vi>=(int)v3Count)       { bad=true; break; }
                    if (tri[k].vt && (vti<0||vti>=(int)v2Count)) { bad=true; break; }
                    t.p [k] = verts3d[vi];
                    t.uv[k] = tri[k].vt
                        ? verts2d[vti]
                        : VEC2D_Vec2dConstructor(0,0);
                }
                if (bad) continue;
                if (VEC3D_TriangleVectorPush(tv, t) != 0) { error = true; break; }
            }
        }
        // else ignore (o, s, mtllib, usemtl, blank, comment…)
    }

    fclose(fp);
    if (error) {
        free(verts3d); free(verts2d);
        VEC3D_TriangleVectorDestroy(tv);
        return NULL;
    }
    free(verts3d); free(verts2d);
    return tv;
}
