#include "Camera.hpp"

#include <gl/glew.h>
#include <gl/freeglut.h>

using namespace glm;

Camera::Camera()
	: Object(), mOrthographic(false), mOrthographicSize(0),
	mFieldOfView(radians(70.f)), mPerspectiveBounds(vec4()),
	mNear(.01f), mFar(100.f),
	mPixelWidth(1600), mPixelHeight(900),
	mColorBuffer(0), mDepthBuffer(0),
	mView(mat4(1.f)), mProjection(mat4(1.f)), mFramebufferDirty(true) {}
Camera::~Camera() {}

void Camera::Set(){
	if (mFramebufferDirty) {
		glGenFramebuffers(1, &mFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
		
		glGenTextures(1, &mColorBuffer);
		glBindTexture(GL_TEXTURE_2D, mColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, mPixelWidth, mPixelHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &mDepthBuffer);
		glBindTexture(GL_TEXTURE_2D, mDepthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, mPixelWidth, mPixelHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorBuffer, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthBuffer, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			printf("Failed to create frameBuffer\n");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		mFramebufferDirty = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);

	glViewport(0, 0, mPixelWidth, mPixelHeight);
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
		else
			mProjection = frustumLH(mPerspectiveBounds.x * mNear, mPerspectiveBounds.y * mNear, mPerspectiveBounds.z * mNear, mPerspectiveBounds.w * mNear, mNear, mFar);
	}

	mViewProjection = mProjection * mView;

	return true;
}