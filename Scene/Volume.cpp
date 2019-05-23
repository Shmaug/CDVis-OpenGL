#include "Volume.hpp"

#pragma warning(disable:26451)

using namespace std;
using namespace glm;

shared_ptr<Shader> Volume::sVolShader = 0;
shared_ptr<Shader> Volume::sComputeShader = 0;
shared_ptr<Mesh> Volume::sMesh = 0;

Volume::Volume()
	: Object(), mDisplaySampleCount(false), mTexture(0), mMask(false), mDirty(true),
	mStepSize(.00135f),
	mDensity(.5f), mThreshold(.2f), mExposure(1.5f), mLightDensity(300.f),
	mPlanePoint(vec3(0.f, -2.f, 0.f)), mPlaneNormal(vec3(0.f, 1.f, 0.f)),
	mLightIntensity(100.0f), mLightAmbient(.2f),
	mLightPosition(vec3(.0f, .1f, 0.f)), mLightDirection(normalize(vec3(-1.0f, -.25f, 0.f))), mLightAngle(.5f) {

	mRenderQueue = 1000;

	if (!sVolShader) {
		sVolShader = shared_ptr<::Shader>(new ::Shader());
		sVolShader->AddShaderFile(GL_VERTEX_SHADER, "Shaders/volume.vert");
		sVolShader->AddShaderFile(GL_FRAGMENT_SHADER, "Shaders/volume.frag");
		sVolShader->CompileAndLink();
	}
	if (!sComputeShader) {
		sComputeShader = shared_ptr<::Shader>(new ::Shader());
		sComputeShader->AddShaderFile(GL_COMPUTE_SHADER, "Shaders/volume.glsl");
		sComputeShader->CompileAndLink();
	}

	// mesh
	if (!sMesh) {
		sMesh = shared_ptr<Mesh>(new Mesh());
		sMesh->BindVAO();

		static const vec3 vertices[8]{
			{-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f},
			{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}
		};
		static const GLuint indices[36]{
			0, 1, 2, 2, 3, 0,
			1, 5, 6, 6, 2, 1,
			7, 6, 5, 5, 4, 7,
			4, 0, 3, 3, 7, 4,
			4, 5, 1, 1, 0, 4,
			3, 2, 6, 6, 7, 3
		};

		sMesh->BindVBO();
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 24, vertices, GL_STATIC_DRAW);

		sMesh->BindIBO();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 36, indices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (GLvoid*)0);

		sMesh->ElementCount(36);
		glBindVertexArray(0);
	}
}
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
		
		mBakedTexture = shared_ptr<::Texture>(new ::Texture(GL_RG16, GL_RG, GL_UNSIGNED_SHORT, mTexture->Width(), mTexture->Height(), mTexture->Depth(), data));
	}

	if (mMask)
		sComputeShader->EnableKeyword("MASK");
	else
		sComputeShader->DisableKeyword("MASK");

	sComputeShader->EnableKeyword("LIGHT_POINT");

	GLuint p = sComputeShader->Use();

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

void Volume::Draw(Camera& camera) {
	if (mDirty) Precompute();

	// draw volume
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	mat4 v2o = inverse(camera.View() * ObjectToWorld());
	mat4 ip = inverse(camera.Projection());
	vec3 lc = (vec3)(WorldToObject() * vec4(camera.WorldPosition(), 1.0));

	if (mDisplaySampleCount)
		sVolShader->EnableKeyword("SAMPLECOUNT");
	else
		sVolShader->DisableKeyword("SAMPLECOUNT");

	GLuint p = sVolShader->Use();

	Shader::Uniform(p, "MVP", camera.Projection() * camera.View() * ObjectToWorld());
	Shader::Uniform(p, "ViewToObject", v2o);
	Shader::Uniform(p, "InverseProjection", ip);
	Shader::Uniform(p, "CameraPosition", lc);

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
	glBindTexture(GL_TEXTURE_2D, camera.DepthBuffer());
	
	sMesh->BindVAO();
	glDrawElements(GL_TRIANGLES, sMesh->ElementCount(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}