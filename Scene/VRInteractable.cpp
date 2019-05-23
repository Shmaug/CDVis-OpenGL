#include "VRInteractable.hpp"

using namespace std;
using namespace glm;

void VRInteractable::Drag(const shared_ptr<Object>& this_obj, const shared_ptr<VRDevice>& device, const vec3& newPos, const quat& newRot) {
	this_obj->LocalPosition(newPos);
	this_obj->LocalRotation(newRot);
}