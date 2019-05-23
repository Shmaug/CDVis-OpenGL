#pragma once

#include <memory>

#include "Object.hpp"
#include "Renderer.hpp"
#include "../Pipeline/Texture.hpp"
#include "../Pipeline/Shader.hpp"
#include "../Pipeline/Mesh.hpp"

class MeshRenderer : public Object, public Renderer {
public:
	bool mVisible;

	MeshRenderer();
	~MeshRenderer();

	inline std::shared_ptr<::Shader> Shader() const { return mShader; }
	inline void Shader(const std::shared_ptr<::Shader>& s) { mShader = s; }

	inline std::shared_ptr<::Mesh> Mesh() const { return mMesh; }
	inline void Mesh(const std::shared_ptr<::Mesh>& m) { mMesh = m; }

	void Uniform(std::string name, int x);
	void Uniform(std::string name, float x);
	void Uniform(std::string name, const glm::vec3& x);
	void Uniform(std::string name, const glm::mat4& x);
	void Uniform(std::string name, const std::shared_ptr<Texture>& x);

	virtual void Draw(Camera& camera);

private:
	struct ShaderUniform {
	public:
		enum Type {
			INT, FLOAT, VEC3, MAT4, TEXTURE
		};
		union Value {
			int intValue;
			float floatValue;
			glm::vec3 vec3Value;
			glm::mat4 mat4Value;
			std::shared_ptr<Texture> textureValue;

			Value() : intValue(0) {}
			~Value() {}
		};

		Value mValue;
		Type mType;

		~ShaderUniform() {
			if (mType == TEXTURE)
				mValue.textureValue.reset();
		}

		ShaderUniform& operator =(const ShaderUniform& x) {
			mType = x.mType;
			switch (mType) {
			case INT:
				mValue.intValue = x.mValue.intValue;
				break;
			case FLOAT:
				mValue.floatValue = x.mValue.floatValue;
				break;
			case VEC3:
				mValue.vec3Value = x.mValue.vec3Value;
				break;
			case MAT4:
				mValue.mat4Value = x.mValue.mat4Value;
				break;
			case TEXTURE:
				mValue.textureValue = x.mValue.textureValue;
				break;
			}
			return *this;
		}

		#define UNBIND_TEX if (mType == TEXTURE) mValue.textureValue.reset();
		inline void set(int x) { UNBIND_TEX mValue.intValue = x; mType = INT; }
		inline void set(float x) { UNBIND_TEX mValue.floatValue = x; mType = FLOAT; }
		inline void set(const glm::vec3& x) { UNBIND_TEX mValue.vec3Value = x; mType = VEC3; }
		inline void set(const glm::mat4& x) { UNBIND_TEX mValue.mat4Value = x; mType = MAT4; }
		inline void set(const std::shared_ptr<Texture>& x) { mValue.textureValue = x; mType = TEXTURE; }
		#undef UNBIND_TEX
	};

	std::shared_ptr<::Mesh> mMesh;
	std::shared_ptr<::Shader> mShader;

	std::unordered_map<std::string, ShaderUniform> mUniforms;
};