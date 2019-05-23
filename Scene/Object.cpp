#include "Object.hpp"

#include <glm/gtx/quaternion.hpp>

using namespace std;
using namespace glm;

Object::Object()
	: mParent(nullptr),
	mLocalPosition(vec3()), mLocalRotation(quat(1.f, 0.f, 0.f, 0.f)), mLocalScale(vec3(1.f, 1.f, 1.f)),
	mWorldPosition(vec3()), mWorldRotation(quat(1.f, 0.f, 0.f, 0.f)), mWorldScale(vec3(1.f, 1.f, 1.f)),
	mObjectToWorld(mat4(1.f)), mWorldToObject(mat4(1.f)), mTransformDirty(true) {
	Dirty();
}
Object::~Object() {
	for (size_t i = 0; i < mChildren.size(); i++)
		if (shared_ptr<Object> o = mChildren[i].lock())
			o->Parent(mParent);
	mChildren.clear();
	Parent(nullptr);
}

bool Object::UpdateTransform() {
	if (!mTransformDirty) return false;

	mObjectToWorld = translate(mat4(1.f), mLocalPosition) * toMat4(mLocalRotation) * scale(mat4(1.f), mLocalScale);

	if (mParent) {
		mObjectToWorld = mParent->ObjectToWorld() * mObjectToWorld;

		mWorldPosition = (vec3)(mParent->mObjectToWorld * vec4(mLocalPosition, 1.f));
		mWorldRotation = mParent->mWorldRotation * mLocalRotation;
		mWorldScale.x = mParent->mWorldScale.x * mLocalScale.x;
		mWorldScale.y = mParent->mWorldScale.y * mLocalScale.y;
		mWorldScale.z = mParent->mWorldScale.z * mLocalScale.z;
	} else {
		mWorldPosition = mLocalPosition;
		mWorldRotation = mLocalRotation;
		mWorldScale = mLocalScale;
	}

	mWorldToObject = inverse(mObjectToWorld);

	mTransformDirty = false;
	return true;
}

void Object::Parent(const shared_ptr<Object>& p) {
	if (mParent == p) return;

	if (mParent)
		for (auto it = mParent->mChildren.begin(); it != mParent->mChildren.end();) {
			shared_ptr<Object> c = (*it).lock();
			if (c && c.get() == this)
				it = mParent->mChildren.erase(it);
			else
				it++;
		}
	mParent = p;
	if (p) p->mChildren.push_back(shared_from_this());
	Dirty();
}

void Object::Dirty() {
	mTransformDirty = true;
	for (unsigned int i = 0; i < mChildren.size(); i++)
		if (shared_ptr<Object> o = mChildren[i].lock())
			o->Dirty();
}