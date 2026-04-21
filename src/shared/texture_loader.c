#define STB_IMAGE_IMPLEMENTATION
#include "texture_loader.h"
#include "stb_image.h"
#include <GL/glu.h>

GLuint load_texture(const char* filepath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image using stb_image
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filepath, &width, &height, &nrChannels, 0);
    
    if (data) {
        // Determine format based on number of channels
        GLenum format = GL_RGB;
        if (nrChannels == 1) {
            format = GL_LUMINANCE;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
        }
        
        // Use gluBuild2DMipmaps for better compatibility
        if (gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data) == 0) {
            printf("Texture loaded: %s (%dx%d, %d channels)\n", filepath, width, height, nrChannels);
        } else {
            printf("Failed to build mipmaps for texture: %s\n", filepath);
        }
    } else {
        printf("Failed to load texture: %s\n", filepath);
        stbi_image_free(data);
        return 0;
    }

    stbi_image_free(data);
    return textureID;
}
