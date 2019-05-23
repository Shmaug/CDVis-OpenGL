#include "MeshRenderer.hpp"

#include "Camera.hpp"

void MeshRenderer::Uniform(std::string name, int x) {
	mUniforms[name].set(x);
}
void MeshRenderer::Uniform(std::string name, float x) {
	mUniforms[name].set(x);
}
void MeshRenderer::Uniform(std::string name, const glm::vec3& x) {
	mUniforms[name].set(x);
}
void MeshRenderer::Uniform(std::string name, const glm::mat4& x) {
	mUniforms[name].set(x);
}
void MeshRenderer::Uniform(std::string name, const std::shared_ptr<Texture>& x) {
	mUniforms[name].set(x);
}

MeshRenderer::MeshRenderer() : mVisible(true) { mRenderQueue = 100; }
MeshRenderer::~MeshRenderer() {}

void MeshRenderer::Draw(Camera& camera) {
	if (!mVisible) return;

	GLuint p = mShader->Use();
	int tex = 0;
	for (const auto& it : mUniforms) {
		switch (it.second.mType) {
		case ShaderUniform::INT:
			glUniform1i(glGetUniformLocation(p, it.first.c_str()), it.second.mValue.intValue);
			break;
		case ShaderUniform::FLOAT:
			glUniform1f(glGetUniformLocation(p, it.first.c_str()), it.second.mValue.floatValue);
			break;
		case ShaderUniform::VEC3:
			glUniform3f(glGetUniformLocation(p, it.first.c_str()), it.second.mValue.vec3Value.x, it.second.mValue.vec3Value.y, it.second.mValue.vec3Value.z);
			break;
		case ShaderUniform::MAT4:
			glUniformMatrix4fv(glGetUniformLocation(p, it.first.c_str()), 1, GL_FALSE, &it.second.mValue.mat4Value[0][0]);
			break;
		case ShaderUniform::TEXTURE:
			glActiveTexture(GL_TEXTURE0 + tex);
			if (it.second.mValue.textureValue->Depth() == 0)
				glBindTexture(GL_TEXTURE_2D, it.second.mValue.textureValue->GLTexture());
			else
				glBindTexture(GL_TEXTURE_3D, it.second.mValue.textureValue->GLTexture());
			glUniform1i(glGetUniformLocation(p, it.first.c_str()), tex);
			tex++;
			break;
		}
	}

	glUniformMatrix4fv(glGetUniformLocation(p, "ObjectToWorld"), 1, GL_FALSE, &ObjectToWorld()[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(p, "ViewProjection"), 1, GL_FALSE, &camera.ViewProjection()[0][0]);

	mMesh->BindVAO();
	glDrawElements(GL_TRIANGLES, mMesh->ElementCount(), GL_UNSIGNED_INT, 0);
}