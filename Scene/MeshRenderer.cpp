#include "MeshRenderer.hpp"

#include <glm/gtx/quaternion.hpp>

#include "../Pipeline/AssetDatabase.hpp"
#include "Camera.hpp"

using namespace std;
using namespace glm;

void MeshRenderer::Uniform(std::string name, int x) {
	mUniforms[name] = x;
}
void MeshRenderer::Uniform(std::string name, float x) {
	mUniforms[name] = x;
}
void MeshRenderer::Uniform(std::string name, const glm::vec2& x) {
	mUniforms[name] = x;
}
void MeshRenderer::Uniform(std::string name, const glm::vec3& x) {
	mUniforms[name] = x;
}
void MeshRenderer::Uniform(std::string name, const glm::vec4& x) {
	mUniforms[name] = x;
}
void MeshRenderer::Uniform(std::string name, const glm::mat4& x) {
	mUniforms[name] = x;
}
void MeshRenderer::Uniform(std::string name, const std::shared_ptr<Texture>& x) {
	mUniforms[name] = x;
}

MeshRenderer::MeshRenderer() : mVisible(true), mDraggable(false), mMesh(0), mShader(0) {}
MeshRenderer::~MeshRenderer() {}

void MeshRenderer::SetUniforms(GLuint p) {
	int tex = 0;
	vec4 v;
	for (const auto& it : mUniforms) {
		switch (it.second.index()) {
		case 0:
			glUniform1i(glGetUniformLocation(p, it.first.c_str()), get<int>(it.second));
			break;
		case 1:
			glUniform1f(glGetUniformLocation(p, it.first.c_str()), get<float>(it.second));
			break;
		case 2:
			v = vec4(get<vec2>(it.second), 0.f, 0.f);
			glUniform2f(glGetUniformLocation(p, it.first.c_str()), v.x, v.y);
			break;
		case 3:
			v = vec4(get<vec3>(it.second), 0.f);
			glUniform3f(glGetUniformLocation(p, it.first.c_str()), v.x, v.y, v.z);
			break;
		case 4:
			v = get<vec4>(it.second);
			glUniform4f(glGetUniformLocation(p, it.first.c_str()), v.x, v.y, v.z, v.w);
			break;
		case 5:
			glUniformMatrix4fv(glGetUniformLocation(p, it.first.c_str()), 1, GL_FALSE, &get<mat4>(it.second)[0][0]);
			break;
		case 6:
		{
			glActiveTexture(GL_TEXTURE0 + tex);
			auto t = get<shared_ptr<Texture>>(it.second);
			if (t->Depth() == 0)
				glBindTexture(GL_TEXTURE_2D, t->GLTexture());
			else
				glBindTexture(GL_TEXTURE_3D, t->GLTexture());
			glUniform1i(glGetUniformLocation(p, it.first.c_str()), tex);
			tex++;
			break;
		}
		}
	}
}

void MeshRenderer::DrawGizmo(Camera& camera) {
	if (!mVisible) return;

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

void MeshRenderer::Draw(Camera& camera) {
	if (!mVisible) return;

	mShader->ClearKeywords();
	for (const auto& kw : mKeywords)
		mShader->EnableKeyword(kw);

	GLuint p = mShader->Use();

	SetUniforms(p);
	
	Shader::Uniform(p, "ObjectToWorld", ObjectToWorld());
	Shader::Uniform(p, "ViewProjection", camera.ViewProjection());

	mMesh->BindVAO();
	glDrawElements(GL_TRIANGLES, mMesh->ElementCount(), GL_UNSIGNED_INT, 0);

	glUseProgram(0);
	glBindVertexArray(0);
}