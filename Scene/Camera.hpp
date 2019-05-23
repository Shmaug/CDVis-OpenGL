#pragma once

#include "Object.hpp"

#include <gl/glew.h>

class Camera : public Object {
public:
	Camera();
	~Camera();

	void Set();

	inline glm::vec4 PerspectiveBounds() const { return mPerspectiveBounds; }
	inline float FieldOfView() const { return mFieldOfView; }
	inline float Near() const { return mNear; }
	inline float Far() const { return mFar; }
	inline unsigned int PixelWidth() const { return mPixelWidth; }
	inline unsigned int PixelHeight() const { return mPixelHeight; }

	inline void PerspectiveBounds(const glm::vec4& f) { mPerspectiveBounds = f; mFieldOfView = 0.f; Dirty(); }
	inline void FieldOfView(float f) { mPerspectiveBounds = glm::vec4(); mFieldOfView = f; Dirty(); }
	inline void Near(float n) { mNear = n; Dirty(); }
	inline void Far(float f) { mFar = f; Dirty(); }
	inline void PixelWidth(unsigned int w) { mPixelWidth = w; mFramebufferDirty = true; Dirty(); }
	inline void PixelHeight(unsigned int h) { mPixelHeight = h; mFramebufferDirty = true; Dirty(); }

	inline glm::mat4 View() { UpdateTransform(); return mView; }
	inline glm::mat4 Projection() { UpdateTransform(); return mProjection; }
	inline glm::mat4 ViewProjection() { UpdateTransform(); return mViewProjection; }

	inline GLuint ColorBuffer() const { return mColorBuffer; }
	inline GLuint DepthBuffer() const { return mDepthBuffer; }

private:
	bool mOrthographic;
	float mOrthographicSize;

	float mFieldOfView;
	float mNear;
	float mFar;
	unsigned int mPixelWidth;
	unsigned int mPixelHeight;

	glm::mat4 mView;
	glm::mat4 mProjection;
	glm::mat4 mViewProjection;
	glm::vec4 mPerspectiveBounds;

	bool mFramebufferDirty;
	GLuint mFrameBuffer;
	GLuint mColorBuffer;
	GLuint mDepthBuffer;

protected:
	bool UpdateTransform() override;
};