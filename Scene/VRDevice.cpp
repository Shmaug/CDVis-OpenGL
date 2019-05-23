#include "VRDevice.hpp"

#include <glm/gtx/matrix_decompose.hpp>

using namespace std;
using namespace glm;

mat4 VR2GL(const vr::HmdMatrix34_t& matPose) {
	return mat4(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
}
void VR2GL(const vr::HmdMatrix34_t& mat, vec3& position, quat& rotation) {
	mat4 m4 = VR2GL(mat);

	vec3 scale;
	vec3 skew;
	vec4 perspective;
	decompose(m4, scale, rotation, position, skew, perspective);

	// rh coordinates to lh coordinates: flip z
	position.z = -position.z;
	rotation.x = -rotation.x;
	rotation.y = -rotation.y;
}

VRDevice::VRDevice(unsigned int index) : mDeviceIndex(index) { mRenderQueue = 200; }
VRDevice::~VRDevice() {}

void VRDevice::TriggerHapticPulse(unsigned short duration) const {
	if (mHmd) mHmd->TriggerHapticPulse(mDeviceIndex, 0, duration);
}

void VRDevice::UpdateDevice(vr::IVRSystem* hmd, const vr::TrackedDevicePose_t& pose) {
	mHmd = hmd;
	mLastDevicePosition = mDevicePosition;
	mLastDeviceRotation = mDeviceRotation;

	VR2GL(pose.mDeviceToAbsoluteTracking, mDevicePosition, mDeviceRotation);

	LocalPosition(mDevicePosition);
	LocalRotation(mDeviceRotation);

	mDeltaDevicePosition = mDevicePosition - mLastDevicePosition;
	// qDelta = qTo * inverse(qFrom)
	mDeltaDeviceRotation = inverse(mLastDeviceRotation) * mDeviceRotation;

	if (hmd->GetTrackedDeviceClass(mDeviceIndex) == vr::TrackedDeviceClass_Controller) {
		mLastState = mState;
		hmd->GetControllerState(mDeviceIndex, &mState, sizeof(vr::VRControllerState_t));
	}
	auto this_controller = dynamic_pointer_cast<VRDevice>(shared_from_this());
	bool grab = false;
	bool activate = false;

	/*
	#pragma region Interaction
	bool grab = ButtonPressed(VRInteraction::GrabButton);
	bool grabStart = ButtonPressedFirst(VRInteraction::GrabButton);
	bool activate = ButtonPressed(VRInteraction::ActivateButton);
	bool activateStart = ButtonPressedFirst(VRInteraction::ActivateButton);


	// interact with objects
	static vector<shared_ptr<Object>> boundsIntersect;
	boundsIntersect.clear();
	GetScene()->Intersect(BoundingSphere(mDevicePosition, .025f), boundsIntersect);

	for (auto it = mHovered.begin(); it != mHovered.end();) {
		if (boundsIntersect.find(dynamic_pointer_cast<Object>(*it)) == -1) {
			(*it)->HoverExit(this_controller);
			it = mHovered.erase(it);
		} else 
			it++;
	}
	for (unsigned int i = 0; i < boundsIntersect.size(); i++) {
		auto g = dynamic_pointer_cast<VRInteractable>(boundsIntersect[i]);
		if (!g) continue;

		if (!mHovered.count(g)) {
			g->HoverEnter(this_controller);
			mHovered.insert(g);
		}

		// track activating this object
		if (activateStart) {
			g->ActivatePress(this_controller);
			mActivated.insert(g);
		}

		// Drag start
		if (grabStart && g->Draggable()) {
			DragOperation* d = nullptr;
			for (unsigned int k = 0; k < mDragging.size(); k++) {
				auto o = mDragging[k].mObject.lock();
				if (!o) {
					mDragging.remove(k);
					k--;
					continue;
				}
				if (dynamic_pointer_cast<Object>(o) == boundsIntersect[i]) {
					d = &mDragging[k];
					break;
				}
			}
			if (!d) {
				d = &mDragging.push_back({});

				XMVECTOR op = XMLoadFloat3(&boundsIntersect[i]->WorldPosition());
				XMVECTOR or = XMLoadFloat4(&boundsIntersect[i]->WorldRotation());

				d->mObject = dynamic_pointer_cast<VRInteractable>(boundsIntersect[i]);
				XMStoreFloat3(&(d->mDragPos), XMVector3Transform(op, XMLoadFloat4x4(&WorldToObject())));
				XMStoreFloat4(&(d->mDragRotation), XMQuaternionMultiply(or , XMQuaternionInverse(XMLoadFloat4(&mDeviceRotation))));

				TriggerHapticPulse(600);
				g->DragStart(this_controller);
			}
		}
	}
	*/

	if (!activate) {
		for (const auto& it : mActivated)
			it->ActivateRelease(this_controller);
		mActivated.clear();
	}

	// apply drag
	for (unsigned int i = 0; i < mDragging.size(); i++) {
		auto obj = mDragging[i].mObject.lock();
		if (!obj) {
			mDragging.erase(mDragging.begin() + i);
			i--;
			continue;
		}

		if (grab) {
			auto object = dynamic_pointer_cast<Object>(obj);
			vec3 pos = (vec3)(ObjectToWorld() * vec4(mDragging[i].mDragPos, 1.f));
			quat rot = mDeviceRotation * mDragging[i].mDragRotation;
			if (auto parent = object->Parent()) {
				pos = (vec3)(parent->WorldToObject() * vec4(pos, 1.f));
				rot = rot * inverse(parent->WorldRotation());
			}
			obj->Drag(object, this_controller, pos, rot);
		} else {
			obj->DragStop(this_controller);
			TriggerHapticPulse(500);
			mDragging.erase(mDragging.begin() + i);
			i--;
		}
	}
	
	#pragma endregion
}

void VRDevice::Draw(Camera& camera) {
	if (mTracking)
		MeshRenderer::Draw(camera);
}