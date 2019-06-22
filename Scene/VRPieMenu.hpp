#pragma once

#include "MeshRenderer.hpp"

class VRPieMenu : public MeshRenderer {
public:
	VRPieMenu(float radius, unsigned int sliceCount, const std::shared_ptr<Texture>& icons);
	~VRPieMenu();

	inline int Hovered() const { return mHoveredSlice; }
	inline int Pressed() const { return mPressedSlice; }
	inline void Pressed(int p) { mPressedSlice = p; }

	bool UpdateTouch(const glm::vec2& touchPos);

	virtual unsigned int RenderQueue() override { return 1100; }

	::Bounds Bounds() override;
	void Draw(Camera& camera) override;

private:
	#pragma pack(push, 1)
	struct PieVertex {
		glm::vec3 pos;
		glm::vec4 col;
		glm::vec2 tex;
	};
	#pragma pack(pop)

	std::vector<PieVertex> mVertices;
	std::vector<GLuint> mIndices;

	unsigned int mSliceCount;
	int mHoveredSlice;
	int mPressedSlice;
	float mRadius;

	void UpdateMesh();
};

