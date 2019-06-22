#include "VRPieMenu.hpp"

#include "../Pipeline/AssetDatabase.hpp"
#include "Camera.hpp"

#pragma warning(disable: 26451)

using namespace std;
using namespace glm;

constexpr unsigned int PieResolution = 64;

VRPieMenu::VRPieMenu(float radius, unsigned int sliceCount, const shared_ptr<Texture>& icons) 
	: MeshRenderer(), mRadius(radius), mSliceCount(sliceCount), mPressedSlice(-1), mHoveredSlice(-1) {
	Mesh(shared_ptr<::Mesh>(new ::Mesh()));
	Shader(AssetDatabase::gPieShader);
	Uniform("Texture", icons);

	UpdateMesh();
}
VRPieMenu::~VRPieMenu() {}

void VRPieMenu::UpdateMesh() {
	unsigned int sliceResolution = (PieResolution + (mSliceCount - 1)) / mSliceCount; // round up so each slice has the same number of triangles

	unsigned int mIndexCount = 3 * (sliceResolution - 1) * mSliceCount;
	unsigned int mIconIndexCount = mSliceCount * 6;

	mVertices.resize(sliceResolution * mSliceCount + 2 + mSliceCount * 4);
	mIndices.resize(mIndexCount + mIconIndexCount);
	
	mVertices[0] = { { 0, 0, 0 }, { 0, 0, 0, .5f }, { 0, 0 } };
	unsigned int i = 0;
	unsigned int j = 1;

	for (unsigned int s = 0; s < mSliceCount; s++) {
		vec4 c = { 0, 0, 0, .75f };
		if (s == mPressedSlice)
			c.x = c.y = c.z = .25f;
		else if (s == mHoveredSlice)
			c.x = c.y = c.z = .1f;

		float to = two_pi<float>() * s / mSliceCount - half_pi<float>();
		for (unsigned int t = 0; t < sliceResolution; t++) {
			float theta = (two_pi<float>() / mSliceCount) * (float)t / (float)(sliceResolution - 1);
			float x = cosf(theta + to);
			float z = sinf(theta + to);

			if (t > 0) {
				mIndices[i++] = 0;
				mIndices[i++] = j;
				mIndices[i++] = j - 1;
			}

			mVertices[j++] = { { x * mRadius, 0, z * mRadius }, c, { x, z } };
		}
	}
	for (unsigned int s = 0; s < mSliceCount; s++) {
		mIndices[i++] = j;
		mIndices[i++] = j + 1;
		mIndices[i++] = j + 2;

		mIndices[i++] = j;
		mIndices[i++] = j + 2;
		mIndices[i++] = j + 3;

		vec4 c = { 1, 1, 1, 1 };
		float y = 0.005f;

		float rp = mRadius * .5f;

		float theta = two_pi<float>() * (s + .5f) / mSliceCount - half_pi<float>();

		float x = cosf(theta) * rp;
		float z = sinf(theta) * rp;

		float rm = rp * cosf(half_pi<float>() - two_pi<float>() / mSliceCount * .5f);
		float b = 0.70710678118f * rm;

		float u0 = (float)s / mSliceCount;
		float u1 = u0 + 1.f / (float)mSliceCount;

		mVertices[j++] = { { x - b, y, z + b }, c, { u0, 0.f } };
		mVertices[j++] = { { x + b, y, z + b }, c, { u1, 0.f } };
		mVertices[j++] = { { x + b, y, z - b }, c, { u1, 1.f } };
		mVertices[j++] = { { x - b, y, z - b }, c, { u0, 1.f } };
	}

	Mesh()->BindVAO();

	Mesh()->BindVBO();
	glBufferData(GL_ARRAY_BUFFER, sizeof(PieVertex) * mVertices.size(), mVertices.data(), GL_STATIC_DRAW);

	Mesh()->BindIBO();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mIndices.size(), mIndices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PieVertex), (void*)offsetof(PieVertex, pos));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(PieVertex), (void*)offsetof(PieVertex, col));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PieVertex), (void*)offsetof(PieVertex, tex));

	Mesh()->ElementCount((int)mIndices.size());
	glBindVertexArray(0);
}
bool VRPieMenu::UpdateTouch(const vec2& touchPos) {
	Uniform("TouchPos", touchPos);

	float angle = atan2f(touchPos.y, touchPos.x) + half_pi<float>();
	if (angle < 0) angle += two_pi<float>();
	if (angle > two_pi<float>()) angle -= two_pi<float>();
	float interval = two_pi<float>() / mSliceCount;
	int s = (int)(angle / interval);
	if (s != mHoveredSlice) {
		mHoveredSlice = s;
		UpdateMesh();
		return true;
	}
	UpdateMesh();
	return false;
}

::Bounds VRPieMenu::Bounds() {
	return ::Bounds(WorldPosition(), vec3(mRadius, mRadius, 0.01f), WorldRotation());
}

void VRPieMenu::Draw(Camera& camera) {
	if (!mVisible) return;

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader()->DisableKeyword("TEXTURED");
	GLuint p = Shader()->Use();
	SetUniforms(p);

	Shader::Uniform(p, "ObjectToWorld", ObjectToWorld());
	Shader::Uniform(p, "ViewProjection", camera.ViewProjection());

	GLsizei v = Mesh()->ElementCount() - mSliceCount * 6;

	Mesh()->BindVAO();
	glDrawElements(GL_TRIANGLES, v, GL_UNSIGNED_INT, nullptr);


	Shader()->EnableKeyword("TEXTURED");
	p = Shader()->Use();
	SetUniforms(p);

	Shader::Uniform(p, "ObjectToWorld", ObjectToWorld());
	Shader::Uniform(p, "ViewProjection", camera.ViewProjection());

	Mesh()->BindVAO();
	glDrawElements(GL_TRIANGLES, mSliceCount * 6, GL_UNSIGNED_INT, (void*)(v * sizeof(GLuint)));

	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}