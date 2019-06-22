#pragma once

#include "MeshRenderer.hpp"
#include "VRDial.hpp"
#include "VRPieMenu.hpp"
#include "Volume.hpp"
#include "VRDevice.hpp"

#include <openvr.h>

#include "VRInteractable.hpp"

class VRInteraction {
public:
	static const vr::EVRButtonId GrabButton = vr::EVRButtonId::k_EButton_Grip;
	static const vr::EVRButtonId PieMenuButton = vr::EVRButtonId::k_EButton_SteamVR_Touchpad;
	static const vr::EVRButtonId ActivateButton = vr::EVRButtonId::k_EButton_SteamVR_Trigger;
	static const vr::EVRButtonId HelpButton = vr::EVRButtonId::k_EButton_ApplicationMenu;

	enum VRTOOL {
		VRTOOL_PLANE,
		VRTOOL_PAINT,
		VRTOOL_ERASE,
		NUM_VRTOOLS
	};

	VRInteraction();
	~VRInteraction();

	void InitTools(std::vector<std::shared_ptr<Object>>& scene);
	void ProcessInput(const std::vector<std::shared_ptr<Object>>& scene, const std::vector<std::shared_ptr<VRDevice>>& controllers, const std::shared_ptr<Volume>& volume, double deltaTime);

	unsigned int mPieMenuAxis;
	VRTOOL mCurTool;

private:
	class HudText : public TextRenderer {
	public:
		HudText() : TextRenderer() {};

		bool Draggable() override { return true; }
	};

	glm::vec3 mLastPenPos;
	std::shared_ptr<MeshRenderer> mPen;
	std::shared_ptr<MeshRenderer> mPlane;
	std::shared_ptr<VRPieMenu> mPieMenu;
	std::shared_ptr<MeshRenderer> mToolTips;
	std::shared_ptr<VRDevice> mToolController;

	std::shared_ptr<VRDial> mThresholdDial;
	std::shared_ptr<VRDial> mDensityDial;
	std::shared_ptr<VRDial> mExposureDial;
};

