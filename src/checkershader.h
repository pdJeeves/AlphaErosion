#ifndef CHECKERSHADER_H
#define CHECKERSHADER_H
#include "glprogram.h"

class ViewWidget;

class CheckerShader : public glProgram
{
public:
    GLint u_windowSize;

    void construct(ViewWidget *gl);
};

extern CheckerShader g_checker;

#endif // CHECKERSHADER_H
