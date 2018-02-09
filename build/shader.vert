#version 150

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
}