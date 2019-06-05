#include "Util.hpp"

#pragma warning(push)
#pragma warning(disable: 6001)
#include <glm/gtx/matrix_decompose.hpp>
#pragma warning(pop)

using namespace glm;

mat4 VR2GL(const vr::HmdMatrix34_t& matPose) {
	return mat4(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
}
mat4 VR2GL(const vr::HmdMatrix44_t& matPose) {
	return mat4(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], matPose.m[3][0],
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], matPose.m[3][1],
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], matPose.m[3][2],
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], matPose.m[3][3]
	);
}
void VR2GL(const vr::HmdMatrix34_t& mat, vec3& position, quat& rotation, bool invert) {
	mat4 m4 = VR2GL(mat);
	if (invert) m4 = inverse(m4);

	vec3 scale;
	vec3 skew;
	vec4 perspective;
	decompose(m4, scale, rotation, position, skew, perspective);

	// rh coordinates to lh coordinates: flip z
	position.z = -position.z;
	rotation.x = -rotation.x;
	rotation.y = -rotation.y;
}