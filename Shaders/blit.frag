#version 460

uniform sampler2D frameTexture;

out vec4 fragColor;
in vec2 texcoord;

void main() {
	fragColor = vec4(texture(frameTexture, texcoord).rgb, 1.0);
}