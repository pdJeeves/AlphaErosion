#include "viewwidget.h"
#include "shader.h"
#include <cstddef>

#define SHADER(k) "#version 150\n" #k

Shader g_shader;

const char g_VertexShader[] =
	"#version 150" "\n"
	"uniform vec2 u_imageSize;" "\n"
	"uniform vec2 u_windowSize;" "\n"
	"\n"
	"uniform float u_fadeInDuration;" "\n"
	"uniform float u_fadeOutStart;" "\n"
	"uniform float u_fadeOutDuration;" "\n"
	"uniform float u_transitionDuration;" "\n"
	"uniform float u_time;" "\n"
	"\n"
	"in vec3 v_vert;" "\n"
	"in vec2 v_uv;" "\n"
	"\n"
	"out vec2 f_uv;" "\n"
	"out vec3 f_life;" "\n"
	"out vec2 f_position;" "\n"
	"\n"
	"void main()" "\n"
	"{" "\n"
	"\t" "f_uv = v_uv;" "\n"
	"\n"
	"\t" "float FadeTransitionStart = u_fadeInDuration + (u_fadeOutStart - u_fadeInDuration) * 0.5f;" "\n"
	"\n"
	"\t" "f_life.r  = u_time / u_fadeInDuration;" "\n"
	"\t" "f_life.g = (u_time - u_fadeOutStart) / u_fadeOutDuration;" "\n"
	"\t" "f_life.b = u_time / max(u_fadeInDuration, u_fadeOutStart + u_fadeOutDuration); " "\n"
//(u_time - FadeTransitionStart) / u_transitionDuration;" "\n"
	"\n"
	"\t" "f_life = clamp(f_life, vec3(0), vec3(1));" "\n"
	"\t" "gl_Position = vec4(v_vert.xy * u_imageSize/u_windowSize, 0, 1.0);" "\n"
	"\t" "f_position  = gl_Position.xy / gl_Position.w + u_windowSize * .5;" "\n"
	"}" "\n";

const char g_FragmentShader[] =
	"#version 150" "\n"
	"uniform sampler2D u_erosion;" "\n"
	"uniform sampler2D u_gradient;" "\n"
	"\n"
	"uniform float u_fadeInDuration;" "\n"
	"uniform float u_fadeOutStart;" "\n"
	"uniform float u_fadeOutDuration;" "\n"
	"uniform float u_transitionDuration;" "\n"
	"uniform float u_time;" "\n"
	"\n"
	"in vec2 f_uv;" "\n"
	"in vec3 f_life;" "\n"
	"in vec2 f_position;" "\n"
	"\n"
	"out vec4 fragColor;" "\n"
	"\n"
	"void main()" "\n"
	"{" "\n"
	"\t" "vec4 texEffect = texture(u_erosion, f_uv);" "\n"
	"\n"
    "\t" "float fadeInFactor = clamp(f_life.r - (1 - texEffect.r), 0, 1);" "\n"
    "\t" "float fadeOutFactor = clamp(texEffect.g - f_life.g, 0, 1);" "\n"
	"\n"
	"\t" "float fadeInStart   = (1-texEffect.r) * u_fadeInDuration;" "\n"
	"\t" "float fadeOutEnd    = texEffect.g * u_fadeOutDuration + u_fadeOutStart;" "\n"
	"\t" "float fadeProgress  = (u_time - fadeInStart) / (fadeOutEnd - fadeInStart);" "\n"
	"\n"
    "\t" "vec2 rampUV = vec2(f_life.r * 0.5f, 1 - fadeInFactor);" "\n"
    "\t" "vec4 texIN = texture(u_gradient, rampUV);" "\n"
	"\n"
    "\t" "rampUV = vec2((0.5f + f_life.g * 0.5f), fadeOutFactor);" "\n"
    "\t" "vec4 texOUT= texture(u_gradient, rampUV);" "\n"
	"\n"
 //   "\t" "vec4 texBLEND = mix(texIN, texOUT, vec4(f_life.b));" "\n"
	"\t" "vec4 texBLEND = texture(u_gradient, vec2(f_life.b, 1-fadeProgress));" "\n"
//	"\t" "vec4 texBLEND = texture(u_gradient, vec2(1-fadeOutFactor, 1-fadeInFactor));" "\n"


	"\n"
	"\t" "fadeInFactor = clamp(fadeInFactor *15 * (1 - texEffect.b), 0, 1);\n"
	"\t" "fadeOutFactor = clamp(fadeOutFactor *15* (1 - texEffect.b), 0, 1);\n"
	//"\t" "fragColor = vec4(fadeProgress, fadeProgress, fadeProgress, 1);" "\n"

   "\t" "fragColor = vec4(texBLEND.rgb, texBLEND.a * fadeInFactor*fadeOutFactor);" "\n"
	"}" "\n";

const char * g_vertexShader() { return g_VertexShader; }
const char * g_fragmentShader() { return g_FragmentShader; }

void Shader::construct(ViewWidget * gl)
{
    if(!tryLoad(gl, "shader.vert", GL_VERTEX_SHADER))
	{
        compile(gl, g_VertexShader, GL_VERTEX_SHADER);
	}

    if(!tryLoad(gl, "shader.frag", GL_FRAGMENT_SHADER))
	{
        compile(gl, g_FragmentShader, GL_FRAGMENT_SHADER);
	}

	attribute(gl, 0, "v_vert");
	attribute(gl, 1, "v_uv");
	link(gl);
	uniform(gl, u_erosion,            "u_erosion");
	uniform(gl, u_gradient,           "u_gradient");
	uniform(gl, u_imageSize,          "u_imageSize");
	uniform(gl, u_windowSize,         "u_windowSize");
	uniform(gl, u_mousePos,           "u_mousePos");
	uniform(gl, u_fadeInDuration,     "u_fadeInDuration");
	uniform(gl, u_fadeOutStart,       "u_fadeOutStart");
	uniform(gl, u_fadeOutDuration,    "u_fadeOutDuration");
	uniform(gl, u_transitionDuration, "u_transitionDuration");
	uniform(gl, u_timeOfDayMs,        "u_timeOfDayMs");
	uniform(gl, u_var,                "u_var");
	uniform(gl, u_time,               "u_time");
}
