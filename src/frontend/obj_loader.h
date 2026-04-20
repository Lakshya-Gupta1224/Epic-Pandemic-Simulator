#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <GL/glut.h>

/* 
 * Compiles the OBJ file at given path into an OpenGL Display List.
 * Returns the GLuint display list ID, or 0 if failed.
 */
GLuint obj_load(const char* filepath);

#endif /* OBJ_LOADER_H */
