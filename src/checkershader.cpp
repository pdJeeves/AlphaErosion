#include "checkershader.h"
#include "viewwidget.h"
#include <cstddef>

#define SHADER(k) "#version 150\n" #k

CheckerShader g_checker;

static const char * kVert();
static const char * kFrag();

void CheckerShader::construct(ViewWidget * gl)
{
    compile(gl, kVert(), GL_VERTEX_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);

    attribute(gl, 0, "v_vert");
    attribute(gl, 1, "v_uv");
    link(gl);
    uniform(gl, u_windowSize,         "u_windowSize");
}

const char * kVert()
{
    return SHADER(
        uniform vec2 u_windowSize;

        in vec3 v_vert;
        in vec2 v_uv;

        out vec2 f_uv;

        void main()
        {
            gl_Position = vec4(v_vert.xy, 0, 1.0);
            f_uv = v_uv * u_windowSize;
        });
}

const char * kFrag()
{
    return SHADER(
    in vec2 f_uv;

    out vec4 fragColor;

    void main()
    {
        int x = int(f_uv.x / 12);
        int y = int(f_uv.y / 12);
        int i = (x ^ y) & 0x01;

        float c = (140 + 80 * i)/255.f;

        fragColor = vec4(c, c, c, 1);
    });
}
