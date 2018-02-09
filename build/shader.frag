#version 150

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
	fragColor = texBLEND * fadeInFactor*fadeOutFactor;
}