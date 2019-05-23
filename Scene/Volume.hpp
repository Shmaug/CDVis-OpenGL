#pragma once

#include <unordered_map>
#include <gl/glew.h>
#include <memory>

#include "Object.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include "../Pipeline/Texture.hpp"
#include "../Pipeline/Shader.hpp"
#include "../Pipeline/Mesh.hpp"

class Volume : public Object, public Renderer {
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

	void Draw(Camera& camera) override;
	void Texture(const std::shared_ptr<::Texture>& tex);

private:
	static std::shared_ptr<Mesh> sMesh;
	static std::shared_ptr<::Shader> sVolShader;
	static std::shared_ptr<::Shader> sComputeShader;

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