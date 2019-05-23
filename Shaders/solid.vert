#version 460

layout(location = 0) in vec3 vertex;

uniform mat4 ObjectToWorld;
uniform mat4 ViewProjection;

void main() {
	gl_Position = ViewProjection * ObjectToWorld * vec4(vertex, 1.0);
}