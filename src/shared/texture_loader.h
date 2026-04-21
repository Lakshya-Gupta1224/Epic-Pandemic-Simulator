#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

GLuint load_texture(const char* filepath);
void init_textures(void);

#endif /* TEXTURE_LOADER_H */
