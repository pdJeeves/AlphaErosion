#ifndef OPENGL_H
#define OPENGL_H

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifdef QT_OPENGL_ES_1
#define GL_ORTHO glOrthof
#else
#define GL_ORTHO glOrtho
#endif

void glPrintErrors(const char * mesg);

#endif // GL_H
