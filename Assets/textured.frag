#version 460

in vs_out {
	vec2 texcoord;
} i;

out vec4 FragColor;

uniform sampler2D Texture;

void main() {
	FragColor = texture(Texture, i.texcoord);
}