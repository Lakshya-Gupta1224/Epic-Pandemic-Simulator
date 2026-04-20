#include "obj_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { float x, y, z; } Vec3;

GLuint obj_load(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Failed to open OBJ file: %s\n", filepath);
        return 0;
    }

    Vec3* vertices = NULL;
    Vec3* normals = NULL;
    int vCount = 0, vCap = 0;
    int nCount = 0, nCap = 0;

    GLuint listId = glGenLists(1);
    glNewList(listId, GL_COMPILE);
    glBegin(GL_TRIANGLES);

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            if (vCount >= vCap) {
                vCap = vCap == 0 ? 1024 : vCap * 2;
                vertices = (Vec3*)realloc(vertices, sizeof(Vec3) * vCap);
            }
            sscanf(line, "v %f %f %f", &vertices[vCount].x, &vertices[vCount].y, &vertices[vCount].z);
            vCount++;
        } else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') {
            if (nCount >= nCap) {
                nCap = nCap == 0 ? 1024 : nCap * 2;
                normals = (Vec3*)realloc(normals, sizeof(Vec3) * nCap);
            }
            sscanf(line, "vn %f %f %f", &normals[nCount].x, &normals[nCount].y, &normals[nCount].z);
            nCount++;
        } else if (line[0] == 'f' && line[1] == ' ') {
            int v[3] = {0}, t[3] = {0}, n[3] = {0};
            int matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                                 &v[0], &t[0], &n[0],
                                 &v[1], &t[1], &n[1],
                                 &v[2], &t[2], &n[2]);
            if (matches != 9) {
                matches = sscanf(line, "f %d//%d %d//%d %d//%d",
                                 &v[0], &n[0],
                                 &v[1], &n[1],
                                 &v[2], &n[2]);
                if (matches != 6) {
                    matches = sscanf(line, "f %d %d %d", &v[0], &v[1], &v[2]);
                }
            }

            for (int i = 0; i < 3; i++) {
                if (n[i] > 0 && n[i] <= nCount) {
                    glNormal3f(normals[n[i]-1].x, normals[n[i]-1].y, normals[n[i]-1].z);
                }
                if (v[i] > 0 && v[i] <= vCount) {
                    glVertex3f(vertices[v[i]-1].x, vertices[v[i]-1].y, vertices[v[i]-1].z);
                }
            }
        }
    }

    glEnd();
    glEndList();

    if (vertices) free(vertices);
    if (normals) free(normals);
    fclose(file);

    return listId;
}
