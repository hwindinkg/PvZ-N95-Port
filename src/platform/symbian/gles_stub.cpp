// gles_stub.cpp - stub implementations of OpenGL ES 1.1 + EGL functions
// (Removes libGLES_CM.dll dependency which may be missing from N95 ROM.
//  PvZ Symbian port uses CFbsBitGc for rendering, not GLES.)

// Plain C stubs - no Symbian includes needed, just use built-in types

// === OpenGL ES 1.1 types ===
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef float GLfloat;
typedef unsigned char GLubyte;
typedef void GLvoid;
typedef int GLfixed;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef float GLclampf;

#define GL_NO_ERROR 0
#define GL_FALSE 0
#define GL_TRUE 1

extern "C" {

// === OpenGL ES 1.1 stubs ===
GLenum glGetError() { return GL_NO_ERROR; }
void glClear(GLbitfield) {}
void glClearColor(GLclampf, GLclampf, GLclampf, GLclampf) {}
void glViewport(GLint, GLint, GLsizei, GLsizei) {}
void glMatrixMode(GLenum) {}
void glLoadIdentity() {}
void glLoadMatrixf(const GLfloat*) {}
void glOrthof(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat) {}
void glEnable(GLenum) {}
void glDisable(GLenum) {}
void glEnableClientState(GLenum) {}
void glScissor(GLint, GLint, GLsizei, GLsizei) {}
void glBindTexture(GLenum, GLuint) {}
void glGenTextures(GLsizei, GLuint*) {}
void glDeleteTextures(GLsizei, const GLuint*) {}
void glTexParameteri(GLenum, GLenum, GLint) {}
void glTexEnvi(GLenum, GLenum, GLint) {}
void glBlendFunc(GLenum, GLenum) {}
void glVertexPointer(GLint, GLenum, GLsizei, const GLvoid*) {}
void glTexCoordPointer(GLint, GLenum, GLsizei, const GLvoid*) {}
void glColorPointer(GLint, GLenum, GLsizei, const GLvoid*) {}
void glDrawArrays(GLenum, GLint, GLsizei) {}
void glDrawTexiOES(GLint, GLint, GLint, GLint, GLint) {}
void glColor4f(GLfloat, GLfloat, GLfloat, GLfloat) {}
void glDisableClientState(GLenum) {}
void glLineWidth(GLfloat) {}
void glTexImage2D(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*) {}
// glMapBufferOES/glUnmapBufferOES are defined as function pointers in GLInterface.cpp

// === EGL types ===
typedef void* EGLDisplay;
typedef void* EGLConfig;
typedef void* EGLContext;
typedef void* EGLSurface;
typedef int EGLint;
typedef unsigned int EGLBoolean;
typedef int EGLNativeDisplayType;
typedef void* EGLNativeWindowType;
typedef void* (*__eglMustCastToProperFunctionPointerType)(void);

#define EGL_FALSE 0
#define EGL_TRUE 1

// === EGL stubs ===
EGLDisplay eglGetDisplay(EGLNativeDisplayType) { return 0; }
EGLBoolean eglInitialize(EGLDisplay, EGLint*, EGLint*) { return EGL_FALSE; }
EGLBoolean eglChooseConfig(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*) { return EGL_FALSE; }
EGLSurface eglCreateWindowSurface(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint*) { return 0; }
EGLContext eglCreateContext(EGLDisplay, EGLConfig, EGLContext, const EGLint*) { return 0; }
EGLBoolean eglMakeCurrent(EGLDisplay, EGLSurface, EGLSurface, EGLContext) { return EGL_FALSE; }
EGLBoolean eglSwapBuffers(EGLDisplay, EGLSurface) { return EGL_FALSE; }
EGLBoolean eglDestroyContext(EGLDisplay, EGLContext) { return EGL_FALSE; }
EGLBoolean eglDestroySurface(EGLDisplay, EGLSurface) { return EGL_FALSE; }
EGLBoolean eglTerminate(EGLDisplay) { return EGL_FALSE; }
__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char*) { return 0; }

} // extern "C"
