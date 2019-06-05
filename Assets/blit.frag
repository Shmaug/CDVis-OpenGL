#version 460

uniform sampler2D FrameTexture;

out vec4 fragColor;
in vec2 texcoord;

void main() {
	fragColor = vec4(texture(FrameTexture, texcoord).rgb, 1.0);
}