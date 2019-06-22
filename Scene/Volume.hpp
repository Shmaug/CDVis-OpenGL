#pragma once

#include <unordered_map>
#include <gl/glew.h>
#include <memory>

#include "Object.hpp"
#include "Camera.hpp"
#include "VRInteractable.hpp"
#include "../Pipeline/Texture.hpp"
#include "../Pipeline/Shader.hpp"
#include "../Pipeline/Mesh.hpp"

class Volume : public Object, public VRInteractable {
public:
	Volume();
	~Volume();

	inline float StepSize() const { return mStepSize; }
	inline bool DisplaySampleCount() const { return mDisplaySampleCount; }
	inline float Density() const { return mDensity; }
	inline float Exposure() const { return mExposure; }
	inline float Threshold() const { return mThreshold; }

	inline void StepSize(float x) { mStepSize = x; }
	inline void DisplaySampleCount(bool x) { mDisplaySampleCount = x; }
	inline void Density(float x) { mDensity = x; mDensity = fmaxf(mDensity, 0.f); mDirty = true; }
	inline void Exposure(float x) { mExposure = x; mExposure = fmaxf(mExposure, 0.f); mDirty = true; }
	inline void Threshold(float x) { mThreshold = x; mThreshold = fminf(fmaxf(mThreshold, 0.f), 1.f); mDirty = true; }

	inline virtual bool Draggable() override { return true; }

	void Texture(const std::shared_ptr<::Texture>& tex);

	::Bounds Bounds() override { return ::Bounds(WorldPosition(), WorldScale() * .5f, WorldRotation()); };
	void Draw(Camera& camera) override;
	void DrawGizmo(Camera& camera) override;

	unsigned int RenderQueue() override { return 5000; }

private:
	bool mDisplaySampleCount;

	bool mMask;
	glm::vec3 mPlanePoint;
	glm::vec3 mPlaneNormal;
	float mExposure;
	float mThreshold;
	float mDensity;
	glm::vec3 mLightPosition;
	glm::vec3 mLightDirection;
	float mLightDensity;
	float mLightIntensity;
	float mLightAmbient;
	float mLightAngle;
	float mStepSize;

	bool mDirty;

	std::shared_ptr<::Texture> mTexture;
	std::shared_ptr<::Texture> mBakedTexture;
	
	void Precompute();

protected:
	virtual bool UpdateTransform() override;
};