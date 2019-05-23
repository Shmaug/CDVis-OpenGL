#pragma once

#include <vector>
#include <openvr.h>

#include "MeshRenderer.hpp"
#include "VRInteractable.hpp"

class VRDevice : public MeshRenderer {
public:
	bool mTracking;

	VRDevice(unsigned int index);
	~VRDevice();

	void TriggerHapticPulse(unsigned short duration) const;

	unsigned int DeviceIndex() const { return mDeviceIndex; }
	const vr::VRControllerState_t& GetState() { return mState; }
	const vr::VRControllerState_t& GetLastState() { return mLastState; }

	inline bool ButtonPressed(vr::EVRButtonId button) const {
		return mState.ulButtonPressed& vr::ButtonMaskFromId(button);
	}
	inline bool ButtonPressedFirst(vr::EVRButtonId button) const {
		return (mState.ulButtonPressed & vr::ButtonMaskFromId(button)) && !(mLastState.ulButtonPressed & vr::ButtonMaskFromId(button));
	}
	inline bool ButtonReleased(vr::EVRButtonId button) const {
		return !(mState.ulButtonPressed & vr::ButtonMaskFromId(button));
	}
	inline bool ButtonReleasedFirst(vr::EVRButtonId button) const {
		return !(mState.ulButtonPressed & vr::ButtonMaskFromId(button)) && (mLastState.ulButtonPressed & vr::ButtonMaskFromId(button));
	}

	inline bool ButtonTouched(vr::EVRButtonId button) const {
		return mState.ulButtonTouched& vr::ButtonMaskFromId(button);
	}
	inline bool ButtonTouchedFirst(vr::EVRButtonId button) const {
		return (mState.ulButtonTouched & vr::ButtonMaskFromId(button)) && !(mLastState.ulButtonTouched & vr::ButtonMaskFromId(button));
	}
	inline bool ButtonTouchReleased(vr::EVRButtonId button) const {
		return !(mState.ulButtonTouched & vr::ButtonMaskFromId(button));
	}
	inline bool ButtonTouchReleasedFirst(vr::EVRButtonId button) const {
		return !(mState.ulButtonTouched & vr::ButtonMaskFromId(button)) && (mLastState.ulButtonTouched & vr::ButtonMaskFromId(button));
	}

	inline glm::vec3 DevicePosition() const { return mDevicePosition; }
	inline glm::quat DeviceRotation() const { return mDeviceRotation; }
	inline glm::vec3 LastDevicePosition() const { return mLastDevicePosition; }
	inline glm::quat LastDeviceRotation() const { return mLastDeviceRotation; }
	inline glm::vec3 DeltaDevicePosition() const { return mDeltaDevicePosition; }
	inline glm::quat DeltaDeviceRotation() const { return mDeltaDeviceRotation; }

	void Draw(Camera& camera) override;
	void UpdateDevice(vr::IVRSystem* hmd, const vr::TrackedDevicePose_t& pose);

private:
	const unsigned int mDeviceIndex;

	vr::IVRSystem* mHmd;

	vr::VRControllerState_t mState;
	vr::VRControllerState_t mLastState;

	glm::vec3 mDevicePosition;
	glm::quat mDeviceRotation;

	glm::vec3 mLastDevicePosition;
	glm::quat mLastDeviceRotation;

	glm::vec3 mDeltaDevicePosition;
	glm::quat mDeltaDeviceRotation;

	struct DragOperation {
		glm::vec3 mDragPos;
		glm::quat mDragRotation;
		std::weak_ptr<VRInteractable> mObject;
	};
	std::vector<DragOperation> mDragging;
	std::unordered_set<std::shared_ptr<VRInteractable>> mHovered;
	std::unordered_set<std::shared_ptr<VRInteractable>> mActivated;
};