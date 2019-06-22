#include "Volume.hpp"

#include <glm/gtx/quaternion.hpp>

#include "../Pipeline/AssetDatabase.hpp"

#pragma warning(disable:26451)

using namespace std;
using namespace glm;

Volume::Volume()
	: Object(), mDisplaySampleCount(false), mTexture(nullptr), mBakedTexture(nullptr), mMask(false), mDirty(true),
	mStepSize(.00135f),
	mDensity(.5f), mThreshold(.2f), mExposure(1.5f), mLightDensity(300.f),
	mPlanePoint(vec3(0.f, -2.f, 0.f)), mPlaneNormal(vec3(0.f, 1.f, 0.f)),
	mLightIntensity(100.0f), mLightAmbient(.2f),
	mLightPosition(vec3(.0f, .1f, 0.f)), mLightDirection(normalize(vec3(-1.0f, -.25f, 0.f))), mLightAngle(.5f) {}
Volume::~Volume() {}

void Volume::Texture(const shared_ptr<::Texture>& tex) {
	mTexture = tex;
	mDirty = true;
}

bool Volume::UpdateTransform() {
	if (!Object::UpdateTransform()) return false;
	mDirty = true;
	return true;
}

void Volume::Precompute() {
	if (!mTexture) return;

	if (!mBakedTexture || mBakedTexture->Width() != mTexture->Width() || mBakedTexture->Height() != mTexture->Height() || mBakedTexture->Depth() != mTexture->Depth()) {
		glEnable(GL_TEXTURE_3D);

		uint16_t* data = new uint16_t[mTexture->Width() * mTexture->Height() * mTexture->Depth() * 2];
		memset(data, 0xFFFF, mTexture->Width() * mTexture->Height() * mTexture->Depth() * 2 * sizeof(uint16_t));
		
		mBakedTexture = shared_ptr<::Texture>(new ::Texture(mTexture->Width(), mTexture->Height(), mTexture->Depth(), GL_RG16, GL_RG, GL_UNSIGNED_SHORT, GL_LINEAR, data));
	}
	
	if (mMask)
		AssetDatabase::gVolumeComputeShader->EnableKeyword("MASK");
	else
		AssetDatabase::gVolumeComputeShader->DisableKeyword("MASK");

	AssetDatabase::gVolumeComputeShader->EnableKeyword("LIGHT_POINT");

	GLuint p = AssetDatabase::gVolumeComputeShader->Use();

	Shader::Uniform(p, "Exposure", mExposure);
	Shader::Uniform(p, "Density", mDensity);
	Shader::Uniform(p, "Threshold", mThreshold);

	Shader::Uniform(p, "WorldScale", LocalScale());
	Shader::Uniform(p, "TexelSize", vec3(1.f / mTexture->Width(), 1.f / mTexture->Height(), 1.f / mTexture->Depth()));

	quat invRot = inverse(WorldRotation());

	Shader::Uniform(p, "LightDensity", mLightDensity);
	Shader::Uniform(p, "LightPosition", invRot * (mLightPosition - WorldPosition()));
	Shader::Uniform(p, "LightDirection", invRot * mLightDirection);
	Shader::Uniform(p, "LightAngle", mLightAngle);
	Shader::Uniform(p, "LightAmbient", mLightAmbient);
	Shader::Uniform(p, "LightIntensity", mLightIntensity);

	glBindImageTexture(0, mTexture->GLTexture(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG16);
	glBindImageTexture(1, mBakedTexture->GLTexture(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16);

	glDispatchCompute((mBakedTexture->Width() + 7) / 8, (mBakedTexture->Height() + 7) / 8, (mBakedTexture->Depth() + 7) / 8);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG16);
	glBindImageTexture(1, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16);

	glUseProgram(0);

	mDirty = false;
}

void Volume::DrawGizmo(Camera& camera) {
	AssetDatabase::gTexturedShader->ClearKeywords();
	AssetDatabase::gTexturedShader->EnableKeyword("NOTEXTURE");

	GLuint p = AssetDatabase::gTexturedShader->Use();

	Shader::Uniform(p, "ObjectToWorld", translate(mat4(1.f), Bounds().mCenter) * toMat4(Bounds().mOrientation) * scale(mat4(1.f), Bounds().mExtents));
	Shader::Uniform(p, "ViewProjection", camera.ViewProjection());
	Shader::Uniform(p, "Color", vec4(1.f, 1.f, 1.f, 1.f));

	AssetDatabase::gWireCubeMesh->BindVAO();
	glDrawElements(GL_LINES, AssetDatabase::gWireCubeMesh->ElementCount(), GL_UNSIGNED_INT, 0);

	glUseProgram(0);
	glBindVertexArray(0);
}

void Volume::Draw(Camera& camera) {
	camera.ResolveDepth(); // so we can access depth texture
	camera.Set();

	if (mDirty) Precompute();

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (mDisplaySampleCount)
		AssetDatabase::gVolumeShader->EnableKeyword("SAMPLECOUNT");
	else
		AssetDatabase::gVolumeShader->DisableKeyword("SAMPLECOUNT");

	GLuint p = AssetDatabase::gVolumeShader->Use();

	Shader::Uniform(p, "MVP", camera.Projection() * camera.View() * ObjectToWorld());
	Shader::Uniform(p, "ViewToObject", inverse(camera.View() * ObjectToWorld()));
	Shader::Uniform(p, "InverseProjection", inverse(camera.Projection()));
	Shader::Uniform(p, "CameraPosition", (vec3)(WorldToObject() * vec4(camera.WorldPosition(), 1.0)));

	Shader::Uniform(p, "PlanePoint", mPlanePoint);
	Shader::Uniform(p, "PlaneNormal", mPlaneNormal);
	Shader::Uniform(p, "StepSize", mStepSize);

	Shader::Uniform(p, "Volume", 0);
	Shader::Uniform(p, "DepthTexture", 1);

	if (mBakedTexture) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, mBakedTexture->GLTexture());
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, camera.ResolveDepthBuffer());
	
	AssetDatabase::gCubeMesh->BindVAO();
	glDrawElements(GL_TRIANGLES, AssetDatabase::gCubeMesh->ElementCount(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}