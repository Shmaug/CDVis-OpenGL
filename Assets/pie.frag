#version 460

#pragma multi_compile TEXTURED

in vs_out {
	vec2 texcoord;
	vec4 color;
} i;

out vec4 FragColor;

uniform vec2 TouchPos;
uniform sampler2D Texture;

void main() {
	FragColor = i.color;
	#ifdef TEXTURED
	FragColor *= texture(Texture, i.texcoord);
	#else
	FragColor.rgb += .5 * pow(1.0 - clamp(length(i.texcoord - TouchPos) * 20.0, 0.0, 1.0), 2.0);
	#endif
	if (FragColor.a < .2) discard;
}