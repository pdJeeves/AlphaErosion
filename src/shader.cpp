#include "viewwidget.h"
#include "shader.h"
#include <cstddef>

#define SHADER(k) "#version 150\n" #k

Shader g_shader;

static const char * kVert();
static const char * kFrag();

void Shader::construct(ViewWidget * gl)
{
    if(!tryLoad(gl, "shader.vert", GL_VERTEX_SHADER))
        compile(gl, kVert(), GL_VERTEX_SHADER);

    if(!tryLoad(gl, "shader.frag", GL_FRAGMENT_SHADER))
        compile(gl, kFrag(), GL_FRAGMENT_SHADER);

	attribute(gl, 0, "v_vert");
	attribute(gl, 1, "v_uv");
	link(gl);
	uniform(gl, u_erosion,            "u_erosion");
	uniform(gl, u_gradient,           "u_gradient");
	uniform(gl, u_imageSize,          "u_imageSize");
	uniform(gl, u_windowSize,         "u_windowSize");
	uniform(gl, u_fadeInDuration,     "u_fadeInDuration");
	uniform(gl, u_fadeOutStart,       "u_fadeOutStart");
	uniform(gl, u_fadeOutDuration,    "u_fadeOutDuration");
	uniform(gl, u_transitionDuration, "u_transitionDuration");
	uniform(gl, u_time,           "u_time");
}

const char * kVert()
{
	return SHADER(
		uniform vec2 u_imageSize;
		uniform vec2 u_windowSize;

		uniform float u_fadeInDuration;
		uniform float u_fadeOutStart;
		uniform float u_fadeOutDuration;
		uniform float u_transitionDuration;
		uniform float u_time;

		in vec3 v_vert;
		in vec2 v_uv;

		out vec2 f_uv;
		out vec3 f_life;

		void main()
		{
			f_uv = v_uv;

			// How much of the blending between the 2 phases is done ? 0 -> 1
			float FadeTransitionStart = u_fadeInDuration + (u_fadeOutStart - u_fadeInDuration) * 0.5f;

			// We store the results in color
			f_life.r  = u_time / u_fadeInDuration;
			f_life.g = (u_time - u_fadeOutStart) / u_fadeOutDuration;
			f_life.b = (u_time - FadeTransitionStart) / u_transitionDuration;

			f_life = clamp(f_life, vec3(0), vec3(1));
			gl_Position = vec4(v_vert.xy * u_imageSize/u_windowSize, 0, 1.0);
		});
}

const char * kFrag()
{
	return SHADER(
	uniform sampler2D u_erosion;
	uniform sampler2D u_gradient;

	in vec2 f_uv;
	in vec3 f_life;

	out vec4 fragColor;

	void main()
	{
		vec4 texEffect = texture(u_erosion, f_uv);

    	float fadeInFactor = clamp(f_life.r - (1 - texEffect.r), 0, 1);
    	float fadeOutFactor = clamp((1-f_life.g) - (1 - texEffect.g), 0, 1);

    	vec2 rampUV = vec2(f_life.r * 0.5f, fadeInFactor);
        vec4 texIN = texture(u_gradient, rampUV);

        rampUV = vec2((0.5f + f_life.g * 0.5f), (1-fadeOutFactor));
        vec4 texOUT= texture(u_gradient, rampUV);

        vec4 texBLEND = mix(texIN, texOUT, vec4(f_life.b));

		fadeInFactor = clamp(fadeInFactor *15 * (1 - texEffect.b), 0, 1);
		fadeOutFactor = clamp(fadeOutFactor *15* (1 - texEffect.b), 0, 1);
        fragColor = vec4(texBLEND.rgb, texBLEND.a * fadeInFactor*fadeOutFactor);
	});
}
