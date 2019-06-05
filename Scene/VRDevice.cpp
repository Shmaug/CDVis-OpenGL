#include "VRDevice.hpp"
#include "../Util/Util.hpp"

#include "VRInteraction.hpp"

#include <set>

using namespace std;
using namespace glm;

VRDevice::VRDevice(unsigned int index) : mDeviceIndex(index), mHmd(nullptr), mTracking(false), mState({}), mLastState({}),
mDevicePosition(vec3()), mDeviceRotation(quat(1.f, 0.f, 0.f, 0.f)),
mDeltaDevicePosition(vec3()), mDeltaDeviceRotation(quat(1.f, 0.f, 0.f, 0.f)),
mLastDevicePosition(vec3()), mLastDeviceRotation(quat(1.f, 0.f, 0.f, 0.f)) {}
VRDevice::~VRDevice() {}

void VRDevice::TriggerHapticPulse(unsigned short duration) const {
	if (mHmd) mHmd->TriggerHapticPulse(mDeviceIndex, 0, duration);
}

void VRDevice::UpdateDevice(const vector<shared_ptr<Object>>& scene, vr::IVRSystem* hmd, const vr::TrackedDevicePose_t& pose) {
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

	#pragma region Interaction
	bool grab = ButtonPressed(VRInteraction::GrabButton);
	bool grabStart = ButtonPressedFirst(VRInteraction::GrabButton);
	bool activate = ButtonPressed(VRInteraction::ActivateButton);
	bool activateStart = ButtonPressedFirst(VRInteraction::ActivateButton);

	// interact with objects
	static unordered_set<shared_ptr<Object>> boundsIntersect;
	boundsIntersect.clear();
	for (const auto& it : scene)
		if (it->Bounds().Intersects(mDevicePosition, .025f))
			boundsIntersect.insert(dynamic_pointer_cast<Object>(it));

	for (auto it = mHovered.begin(); it != mHovered.end();) {
		if (boundsIntersect.count(dynamic_pointer_cast<Object>(*it)) == 0) {
			(*it)->HoverExit(this_controller);
			it = mHovered.erase(it);
		} else 
			it++;
	}
	for (auto& i : boundsIntersect) {
		auto g = dynamic_pointer_cast<VRInteractable>(i);
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
			for (auto& it = mDragging.begin(); it != mDragging.end();) {
				auto o = (*it).mObject.lock();
				if (!o) {
					it = mDragging.erase(it);
					continue;
				}
				if (dynamic_pointer_cast<Object>(o) == i) {
					d = &(*it);
					break;
				}
				it++;
			}
			if (!d) {
				mDragging.push_back({});
				d = &mDragging[mDragging.size() - 1];

				vec3 op = i->WorldPosition();
				quat or = i->WorldRotation();

				d->mObject = dynamic_pointer_cast<VRInteractable>(i);
				d->mDragPos = WorldToObject() * vec4(op, 1.f);
				d->mDragRotation = or * inverse(mDeviceRotation);

				TriggerHapticPulse(600);
				g->DragStart(this_controller);
			}
		}
	}

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