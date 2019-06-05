#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <memory>

#include "../Util/Bounds.hpp"

class Camera;

class Object : public std::enable_shared_from_this<Object> {
public:
	Object();
	~Object();

	std::shared_ptr<Object> Parent() const { return mParent; }
	void Parent(const std::shared_ptr<Object>& obj);

	inline glm::vec3 WorldPosition() { UpdateTransform(); return mWorldPosition; }
	inline glm::quat WorldRotation() { UpdateTransform(); return mWorldRotation; }
	inline glm::vec3 WorldScale() { UpdateTransform(); return mWorldScale; }

	inline glm::vec3 LocalPosition() { UpdateTransform(); return mLocalPosition; }
	inline glm::quat LocalRotation() { UpdateTransform(); return mLocalRotation; }
	inline glm::vec3 LocalScale() { UpdateTransform(); return mLocalScale; }

	inline glm::mat4 ObjectToWorld() { UpdateTransform(); return mObjectToWorld; }
	inline glm::mat4 WorldToObject() { UpdateTransform(); return mWorldToObject; }

	inline void LocalPosition(glm::vec3 p) { mLocalPosition = p; Dirty(); }
	inline void LocalRotation(glm::quat r) { mLocalRotation = r; Dirty(); }
	inline void LocalScale(glm::vec3 s) { mLocalScale = s; Dirty(); }

	inline void LocalPosition(float x, float y, float z) { mLocalPosition.x = x; mLocalPosition.y = y; mLocalPosition.z = z; Dirty(); }
	inline void LocalScale(float x, float y, float z) { mLocalScale.x = x; mLocalScale.y = y; mLocalScale.z = z; Dirty(); }

	virtual unsigned int RenderQueue();
	virtual void Draw(Camera& camera);
	virtual void DrawGizmo(Camera& camera);
	virtual Bounds Bounds();

private:
	bool mTransformDirty;
	glm::vec3 mLocalPosition;
	glm::quat mLocalRotation;
	glm::vec3 mLocalScale;
	glm::mat4 mObjectToWorld;
	glm::mat4 mWorldToObject;

	glm::vec3 mWorldPosition;
	glm::quat mWorldRotation;
	glm::vec3 mWorldScale;

	std::shared_ptr<Object> mParent;
	std::vector<std::weak_ptr<Object>> mChildren;

protected:
	void Dirty();
	virtual bool UpdateTransform();
};