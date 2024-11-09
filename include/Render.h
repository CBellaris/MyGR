#include <glad/glad.h>
#include <iostream>

#define GL_CALL(func) \
    func; \
    checkGLError(__FILE__, __LINE__)

void checkGLError(const char* file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error at " << file << ":" << line << " - " << error << std::endl;
    }
}

