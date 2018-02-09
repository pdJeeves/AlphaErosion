#include "opengl.h"
#include <cstdio>

void glPrintErrors(const char * mesg)
{
	for(GLenum err; (err = glGetError()) != GL_NO_ERROR;)
		fprintf(stderr, "%s: %s!\n", mesg, (char*)gluErrorString(err));
}
