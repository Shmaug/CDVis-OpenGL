#include "Camera.hpp"

#include <gl/glew.h>

#include "../Pipeline/Shader.hpp"

using namespace std;
using namespace glm;

unsigned int Camera::sCameraCount = 0;
shared_ptr<Shader> Camera::sGizmoShader = 0;

Camera::Camera() : Object(),
	mOrthographic(false), mOrthographicSize(0),
	mFieldOfView(radians(70.f)), mPerspectiveBounds(vec4(0.f)),
	mNear(.01f), mFar(50.f),
	mPixelWidth(1600), mPixelHeight(900),
	mColorBuffer(0), mDepthBuffer(0), mResolveColorBuffer(0), mResolveDepthBuffer(0), mResolveFrameBuffer(0), mFrameBuffer(0), mSampleCount(4),
	mView(mat4(1.f)), mProjection(mat4(1.f)), mViewProjection(mat4(1.f)), mFramebufferDirty(true),
	mGizmoMesh(0) {

	sCameraCount++;
	if (!sGizmoShader) {
		sGizmoShader = shared_ptr<::Shader>(new ::Shader());
		sGizmoShader->AddShaderSource(GL_VERTEX_SHADER,
			"#version 460\n"
			"layout(location = 0) in vec3 vertex;"
			"uniform mat4 ObjectToWorld;"
			"uniform mat4 ViewProjection;"
			"void main() { gl_Position = ViewProjection * ObjectToWorld * vec4(vertex, 1.0); }"
		);
		sGizmoShader->AddShaderFile(GL_FRAGMENT_SHADER,
			"#version 460\n"
			"uniform vec4 Color;"
			"out vec4 FragColor;"
			"void main() { FragColor = Color; }"
		);
		sGizmoShader->CompileAndLink();
	}
}
Camera::~Camera() {
	sCameraCount--;
	if (sCameraCount == 0) sGizmoShader.reset();
	
	glDeleteFramebuffers(1, &mFrameBuffer);
	glDeleteFramebuffers(1, &mResolveFrameBuffer);
	glDeleteTextures(1, &mColorBuffer);
	glDeleteTextures(1, &mDepthBuffer);
	glDeleteTextures(1, &mResolveColorBuffer);
	glDeleteTextures(1, &mResolveDepthBuffer);
}

void Camera::Set() {
	if (mFramebufferDirty) {
		if (!mFrameBuffer) glGenFramebuffers(1, &mFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);

		if (!mColorBuffer) glGenTextures(1, &mColorBuffer);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mColorBuffer);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mSampleCount, GL_RGBA8, mPixelWidth, mPixelHeight, true);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mColorBuffer, 0);

		if (!mDepthBuffer) glGenRenderbuffers(1, &mDepthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, mSampleCount, GL_DEPTH_COMPONENT32, mPixelWidth, mPixelHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);

		if (!mResolveFrameBuffer) glGenFramebuffers(1, &mResolveFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, mResolveFrameBuffer);

		if (!mResolveColorBuffer) glGenTextures(1, &mResolveColorBuffer);
		glBindTexture(GL_TEXTURE_2D, mResolveColorBuffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mPixelWidth, mPixelHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mResolveColorBuffer, 0);

		glGenTextures(1, &mResolveDepthBuffer);
		glBindTexture(GL_TEXTURE_2D, mResolveDepthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, mPixelWidth, mPixelHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mResolveDepthBuffer, 0);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			printf("Failed to create framebuffer!\n");

		mFramebufferDirty = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
	glViewport(0, 0, mPixelWidth, mPixelHeight);

	if (mSampleCount)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);
}

void Camera::Resolve() {
	glDisable(GL_MULTISAMPLE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mFrameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mResolveFrameBuffer);

	glBlitFramebuffer(0, 0, mPixelWidth, mPixelHeight, 0, 0, mPixelWidth, mPixelHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
void Camera::ResolveDepth() {
	glDisable(GL_MULTISAMPLE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mFrameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mResolveFrameBuffer);

	glBlitFramebuffer(0, 0, mPixelWidth, mPixelHeight, 0, 0, mPixelWidth, mPixelHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

bool Camera::UpdateTransform() {
	if (!Object::UpdateTransform()) return false;

	vec3 up = WorldRotation() * vec3(0.f, 1.f, 0.f);
	vec3 fwd = WorldRotation() * vec3(0.f, 0.f, 1.f);

	mView = lookAtLH(WorldPosition(), WorldPosition() + fwd, up);

	float aspect = (float)mPixelWidth / (float)mPixelHeight;

	if (mOrthographic)
		mProjection = orthoLH(-mOrthographicSize / aspect, mOrthographicSize / aspect, -mOrthographicSize, mOrthographicSize, mNear, mFar);
	else {
		if (mFieldOfView)
			mProjection = perspectiveLH(mFieldOfView, aspect, mNear, mFar);
		else {
			vec4 s = mPerspectiveBounds * mNear;
			mProjection = frustumLH(s.x, s.y, s.z, s.w, mNear, mFar);
		}
	}

	mViewProjection = mProjection * mView;
	
	mat4 invVP = inverse(mViewProjection);
	vec3 vertices[]{
		vec3(-1.f,  1.f, -1.f),
		vec3(-1.f,  1.f,  1.f),
		vec3( 1.f,  1.f,  1.f),
		vec3( 1.f,  1.f, -1.f),
		
		vec3(-1.f, -1.f, -1.f),
		vec3(-1.f, -1.f,  1.f),
		vec3( 1.f, -1.f,  1.f),
		vec3( 1.f, -1.f, -1.f)
	};

	for (int i = 0; i < 8; i++) {
		vec4 p = invVP * vec4(vertices[i], 1.f);
		p /= p.w;
		vertices[i] = (vec3)p;
	}

	bool genIndices = false;
	if (!mGizmoMesh) {
		mGizmoMesh = shared_ptr<Mesh>(new Mesh());
		genIndices = true;
	}

	mGizmoMesh->BindVAO();
	mGizmoMesh->BindVBO();
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 24, vertices, GL_STATIC_DRAW);

	if (genIndices) {
		GLuint indices[]{
			0, 1, 1, 2, 2, 3, 3, 0,
			4, 5, 5, 6, 6, 7, 7, 4,
			0, 4, 1, 5, 2, 6, 3, 7
		};

		mGizmoMesh->BindIBO();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 24, indices, GL_STATIC_DRAW);
		mGizmoMesh->ElementCount(24);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (GLvoid*)0);
	}
	glBindVertexArray(0);

	return true;
}

void Camera::DrawGizmo(Camera& camera) {
	if (!mGizmoMesh) return;

	GLuint p = sGizmoShader->Use();

	sGizmoShader->Uniform(p, "ObjectToWorld", mat4(1.f));
	sGizmoShader->Uniform(p, "ViewProjection", camera.ViewProjection());
	sGizmoShader->Uniform(p, "Color", vec4(1.f));

	mGizmoMesh->BindVAO();
	glDrawElements(GL_LINES, mGizmoMesh->ElementCount(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}