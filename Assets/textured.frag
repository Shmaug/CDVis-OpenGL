#version 460

#pragma multi_compile NOTEXTURE

in vs_out {
	vec2 texcoord;
} i;

out vec4 FragColor;

#ifndef NOTEXTURE
uniform sampler2D Texture;
#endif
uniform vec4 Color;

void main() {
	FragColor = Color;
	#ifndef NOTEXTURE
	FragColor *= texture(Texture, i.texcoord);
	#endif
}