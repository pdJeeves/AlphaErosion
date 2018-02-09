#ifndef SHADER_H
#define SHADER_H
#include "glprogram.h"

class ViewWidget;

class Shader : public glProgram
{
public:
	GLint u_erosion;
	GLint u_gradient;
	GLint u_imageSize;
	GLint u_windowSize;
	GLint u_fadeInDuration;
	GLint u_fadeOutStart;
	GLint u_fadeOutDuration;
	GLint u_transitionDuration;
	GLint u_time;

    void construct(ViewWidget *gl);
};

extern Shader g_shader;

#endif // SHADER_H
