// loadFromObjectFile.c
#include "../include/loadFromObjectFile.h"

TriangleVector loadFromObjectFile(const char* fileName) {
    FILE* fp = fopen(fileName, "r");
    if (!fp) return NULL;

    Vec3d*   verts  = NULL;
    size_t   vCount = 0, vCap = 0;
    TriangleVector tv = VEC3D_TriangleVectorCreate();
    if (!tv) {
        fclose(fp);
        return NULL;
    }

    char  line[256];
    bool  error = false;

    while (!error && fgets(line, sizeof line, fp)) {
        char* p = line;
        // skip leading whitespace
        while (*p == ' ' || *p == '\t') p++;

        // Vertex?
        if (*p == 'v' && (p[1] == ' ' || p[1] == '\t')) {
            float x,y,z;
            if (sscanf(p+1, "%f %f %f", &x, &y, &z) == 3) {
                // grow if needed
                if (vCount == vCap) {
                    size_t newCap = vCap ? vCap * 2 : 8;
                    Vec3d* tmp = realloc(verts, newCap * sizeof *tmp);
                    if (!tmp) { error = true; break; }
                    verts = tmp;
                    vCap  = newCap;
                }
                verts[vCount++] =  VEC3D_Vec3dConstructor(x,y,z);
            }
        }
        // Face?
        else if (*p == 'f' && (p[1] == ' ' || p[1] == '\t')) {
            // tokenize indices
            int   idx[32], count = 0;
            char* tok = strtok(p+1, " \t\r\n");
            while (tok && count < 32) {
                int vi = atoi(tok);
                if (vi) idx[count++] = vi;
                tok = strtok(NULL, " \t\r\n");
            }
            // fan‑triangulate
            for (int i = 1; i + 1 < count; i++) {
                int i0 = idx[0] < 0 ? (int)vCount + idx[0] : idx[0] - 1;
                int i1 = idx[i] < 0   ? (int)vCount + idx[i] : idx[i] - 1;
                int i2 = idx[i+1] < 0 ? (int)vCount + idx[i+1] : idx[i+1] - 1;
                // bounds check
                if (i0 < 0 || i0 >= (int)vCount ||
                    i1 < 0 || i1 >= (int)vCount ||
                    i2 < 0 || i2 >= (int)vCount) {
                    continue;
                }
                Triangle t = {
                    .p[0]  = verts[i0],
                    .p[1]  = verts[i1],
                    .p[2]  = verts[i2],
                    .shade = 255
                };
                if (VEC3D_TriangleVectorPush(tv, t) != 0) {
                    error = true;
                    break;
                }
            }
        }
        // else ignore other lines
    }

    fclose(fp);

    if (error) {
        free(verts);
        VEC3D_TriangleVectorDestroy(tv);
        return NULL;
    }

    free(verts);
    return tv;
}
