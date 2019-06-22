#pragma once

#include <memory>
#include <variant>

#include "Object.hpp"
#include "../Pipeline/Texture.hpp"
#include "../Pipeline/Shader.hpp"
#include "../Pipeline/Mesh.hpp"
#include "VRInteractable.hpp"

class MeshRenderer : public Object, public VRInteractable {
public:
	bool mVisible;
	bool mDraggable;

	MeshRenderer();
	~MeshRenderer();

	inline std::shared_ptr<::Shader> Shader() const { return mShader; }
	inline void Shader(const std::shared_ptr<::Shader>& s) { mShader = s; }

	inline void EnableKeyword(const std::string& keyword) { mKeywords.insert(keyword); }
	inline void DisableKeyword(const std::string& keyword) { mKeywords.erase(keyword); }

	inline std::shared_ptr<::Mesh> Mesh() const { return mMesh; }
	inline void Mesh(const std::shared_ptr<::Mesh>& m) { mMesh = m; }

	void Uniform(std::string name, int x);
	void Uniform(std::string name, float x);
	void Uniform(std::string name, const glm::vec2& x);
	void Uniform(std::string name, const glm::vec3& x);
	void Uniform(std::string name, const glm::vec4& x);
	void Uniform(std::string name, const glm::mat4& x);
	void Uniform(std::string name, const std::shared_ptr<Texture>& x);

	void SetUniforms(GLuint shader);
	virtual void DrawGizmo(Camera& camera) override;
	virtual void Draw(Camera& camera) override;
	inline virtual ::Bounds Bounds() override {
		return mMesh ?
			::Bounds((glm::vec3)(ObjectToWorld() * glm::vec4(mMesh->Bounds().mCenter, 1.f)), mMesh->Bounds().mExtents, WorldRotation()) :
			::Bounds(WorldPosition(), glm::vec3(), WorldRotation());
	}
	inline virtual unsigned int RenderQueue() override { return 1000; }

	inline virtual bool Draggable() override { return mDraggable; }

private:
	std::shared_ptr<::Mesh> mMesh;
	std::shared_ptr<::Shader> mShader;
	std::unordered_set<std::string> mKeywords;

	std::unordered_map<std::string, std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4, std::shared_ptr<Texture>>> mUniforms;
};