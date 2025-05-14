// loadFromObjectFile.c
#include "../include/loadFromObjectFile.h"

static Vec3d *growV3(Vec3d *buf, size_t *cap)
{
    size_t newCap = *cap ? *cap * 2 : 8;
    Vec3d *tmp    = realloc(buf, newCap * sizeof *tmp);
    if (tmp) *cap = newCap;
    return tmp;
}
static Vec2d *growV2(Vec2d *buf, size_t *cap)
{
    size_t newCap = *cap ? *cap * 2 : 8;
    Vec2d *tmp    = realloc(buf, newCap * sizeof *tmp);
    if (tmp) *cap = newCap;
    return tmp;
}

typedef struct { int v, vt; } FaceIdx;

TriangleVector loadFromObjectFile(const char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (!fp) return NULL;
    Vec3d *verts3d = NULL;  size_t v3Count = 0, v3Cap = 0;
    Vec2d *verts2d = NULL;  size_t v2Count = 0, v2Cap = 0;
    TriangleVector tv = VEC3D_TriangleVectorCreate();
    if (!tv) { fclose(fp); return NULL; }

    char line[512];
    bool error = false;
    while (!error && fgets(line, sizeof line, fp))
    {
        char *p = line;
        while (*p == ' ' || *p == '\t') ++p;          
        if (*p == 'v' && (p[1] == ' ' || p[1] == '\t'))
        {
            float x, y, z;
            if (sscanf(p + 1, "%f %f %f", &x, &y, &z) == 3) {
                if (v3Count == v3Cap && !(verts3d = growV3(verts3d, &v3Cap))) {
                    error = true; break;
                }
                verts3d[v3Count++] = VEC3D_Vec3dConstructor(x, y, z);
            }
        }
        else if (*p == 'v' && p[1] == 't' && (p[2] == ' ' || p[2] == '\t'))
        {
            float u, v;
            if (sscanf(p + 2, "%f %f", &u, &v) == 2) {
                if (v2Count == v2Cap && !(verts2d = growV2(verts2d, &v2Cap))) {
                    error = true; break;
                }
                verts2d[v2Count++] = VEC2D_Vec2dConstructor(u, v);
            }
        }
        else if (*p == 'f' && isspace((unsigned char)p[1]))
        {
            FaceIdx idx[64];
            int     count = 0;
            for (char *tok = strtok(p + 1, " \t\r\n");
                tok && count < (int)(sizeof idx / sizeof *idx);
                tok = strtok(NULL, " \t\r\n"))
            {
                int v  = 0, vt = 0;
                char *s = tok;
                v = (int)strtol(s, &s, 10);          
                if (*s == '/') {                    
                    ++s;
                    vt = (int)strtol(s, &s, 10);
                }
                idx[count++] = (FaceIdx){ v, vt };
            }
            for (int i = 1; i + 1 < count; ++i) {
                Triangle t = { .shade = 255 };
                int trip[3][2] = {
                    { idx[0].v,   idx[0].vt   },
                    { idx[i].v,   idx[i].vt   },
                    { idx[i+1].v, idx[i+1].vt }
                };
                bool bad = false;
                for (int k = 0; k < 3; ++k){
                    int vi  = trip[k][0] < 0 ? (int)v3Count + trip[k][0] : trip[k][0] - 1;
                    int vti = trip[k][1] < 0 ? (int)v2Count + trip[k][1] : trip[k][1] - 1;

                    if (vi  < 0 || vi  >= (int)v3Count) { bad = true; break; }
                    if (trip[k][1] &&            /* only check if /vt present */
                        (vti < 0 || vti >= (int)v2Count)) { bad = true; break; }

                    t.p [k] = verts3d[vi];
                    t.uv[k] = trip[k][1] ? verts2d[vti] : (Vec2d){0.f, 0.f, 1.f};
                }
                if (bad) continue;
                if (VEC3D_TriangleVectorPush(tv, t) != 0) { error = true; break; }
            }
        }
        /* else ignore other line types */
    }
    fclose(fp);
    if (error) {
        free(verts3d);free(verts2d);
        VEC3D_TriangleVectorDestroy(tv);
        return NULL;
    }
    free(verts3d);free(verts2d);
    return tv;
}