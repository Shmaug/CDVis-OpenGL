#version 460

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texcoord;

out vs_out {
	vec2 texcoord;
	vec4 color;
} o;

uniform mat4 ObjectToWorld;
uniform mat4 ViewProjection;

void main() {
	gl_Position = ViewProjection * ObjectToWorld * vec4(vertex, 1.0);
	o.texcoord = texcoord;
	o.color = color;
}