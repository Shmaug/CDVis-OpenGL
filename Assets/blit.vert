#version 460

layout(location = 0) in vec2 vertex;

out vec2 texcoord;

uniform vec4 ScaleOffset;

void main(){
	gl_Position = vec4(vertex * ScaleOffset.xy + ScaleOffset.zw, 0.0, 1.0);
	texcoord = vertex;
	texcoord.y = -texcoord.y;
	texcoord = vertex.xy * .5 + .5;
}