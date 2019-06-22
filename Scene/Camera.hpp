#pragma once

#include "Object.hpp"
#include "../Pipeline/Texture.hpp"
#include "../Pipeline/Mesh.hpp"
#include "../Pipeline/Shader.hpp"

#include <gl/glew.h>

class Camera : public Object {
public:
	Camera();
	~Camera();

	void Set();
	void Resolve();
	void ResolveDepth();

	inline float FieldOfView() const { return mFieldOfView; }
	inline float Near() const { return mNear; }
	inline float Far() const { return mFar; }
	inline unsigned int PixelWidth() const { return mPixelWidth; }
	inline unsigned int PixelHeight() const { return mPixelHeight; }
	inline unsigned int SampleCount() const { return mSampleCount; }

	inline void PerspectiveBounds(const glm::vec4& p) { mPerspectiveBounds = p; mFieldOfView = 0.f; Dirty(); }
	inline void FieldOfView(float f) { mPerspectiveBounds = glm::vec4(0.f); mFieldOfView = f; Dirty(); }
	inline void Near(float n) { mNear = n; Dirty(); }
	inline void Far(float f) { mFar = f; Dirty(); }
	inline void PixelWidth(unsigned int w) { mPixelWidth = w; Dirty(); mFramebufferDirty = true; }
	inline void PixelHeight(unsigned int h) { mPixelHeight = h; Dirty(); mFramebufferDirty = true; }
	inline void SampleCount(unsigned int s) { mSampleCount = s; Dirty(); mFramebufferDirty = true; }

	inline glm::mat4 View() { UpdateTransform(); return mView; }
	inline glm::mat4 Projection() { UpdateTransform(); return mProjection; }
	inline glm::mat4 ViewProjection() { UpdateTransform(); return mViewProjection; }

	inline GLuint ColorBuffer() const { return mColorBuffer; }
	inline GLuint DepthBuffer() const { return mDepthBuffer; }
	inline GLuint ResolveColorBuffer() const { return mResolveColorBuffer; }
	inline GLuint ResolveDepthBuffer() const { return mResolveDepthBuffer; }

private:
	bool mOrthographic;
	float mOrthographicSize;

	float mFieldOfView;
	float mNear;
	float mFar;
	unsigned int mPixelWidth;
	unsigned int mPixelHeight;
	unsigned int mSampleCount;
	glm::vec4 mPerspectiveBounds;

	glm::mat4 mView;
	glm::mat4 mProjection;
	glm::mat4 mViewProjection;

	std::shared_ptr<Mesh> mGizmoMesh;

	bool mFramebufferDirty;
	GLuint mFrameBuffer;
	GLuint mResolveFrameBuffer;
	GLuint mColorBuffer;
	GLuint mDepthBuffer;
	GLuint mResolveColorBuffer;
	GLuint mResolveDepthBuffer;

protected:
	virtual bool UpdateTransform() override;
	virtual void DrawGizmo(Camera& camera) override;
};