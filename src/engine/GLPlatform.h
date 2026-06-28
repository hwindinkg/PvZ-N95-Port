// GLPlatform.h
// Platform-specific GLES 1.1 initialization for Symbian S60 3rd FP1
// C++03 compatible, no exceptions, no STL

#ifndef GLPLATFORM_H
#define GLPLATFORM_H

#include <gles/gl.h>
#include <gles/egl.h>

// Optional extension function pointer types for GLES 1.1 on Symbian.
// These are declared here so PlatformGLInit can load them at runtime.
// Not all Symbian GLES drivers export every extension, so we query
// via eglGetProcAddress.

// GL_OES_draw_texture extension (useful for 2D sprite blits)
typedef void (GL_APIENTRY* PFNGLDRAWTEXIOES)(GLint x, GLint y, GLint z, GLint w, GLint h);
// GL_OES_texture_npot / sub-image
typedef void (GL_APIENTRY* PFNGLTEXSUBIMAGE2DOES)(GLenum target, GLint level,
    GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
    GLenum format, GLenum type, const GLvoid* pixels);
// GL_OES_mapbuffer
typedef void* (GL_APIENTRY* PFNGLMAPBUFFEROES)(GLenum target, GLenum access);
typedef GLboolean (GL_APIENTRY* PFNGLUNMAPBUFFEROES)(GLenum target);

// Loaded extension function pointers (NULL if unavailable)
// glDrawTexiOES is built-into Symbian GLES driver
    // extern PFNGLDRAWTEXIOES       glDrawTexiOES;
extern PFNGLTEXSUBIMAGE2DOES  glTexSubImage2DOES;
extern PFNGLMAPBUFFEROES      glMapBufferOES;
extern PFNGLUNMAPBUFFEROES    glUnmapBufferOES;

// PlatformGLInit()
// Called once during GLES initialization to load any optional extensions.
// No glad, no SDL -- pure eglGetProcAddress.
inline void PlatformGLInit()
{
    // Query GL_OES_draw_texture
    // glDrawTexiOES = (PFNGLDRAWTEXIOES)eglGetProcAddress("glDrawTexiOES"); // built-in

    // Query GL_OES_texture_npot sub-image
    glTexSubImage2DOES = (PFNGLTEXSUBIMAGE2DOES)eglGetProcAddress("glTexSubImage2DOES");

    // Query GL_OES_mapbuffer (VBO support)
    glMapBufferOES   = (PFNGLMAPBUFFEROES)eglGetProcAddress("glMapBufferOES");
    glUnmapBufferOES = (PFNGLUNMAPBUFFEROES)eglGetProcAddress("glUnmapBufferOES");
}

#endif // GLPLATFORM_H
