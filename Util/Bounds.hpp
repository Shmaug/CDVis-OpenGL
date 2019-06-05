#ifndef BOUNDS_HPP
#define BOUNDS_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct AABB {
	glm::vec3 mCenter;
	glm::vec3 mExtents;

	AABB() : mCenter(glm::vec3()), mExtents(glm::vec3()) {}
	AABB(const glm::vec3& center, const glm::vec3& extents) : mCenter(center), mExtents(extents) {}
	AABB(const AABB& aabb) : mCenter(aabb.mCenter), mExtents(aabb.mExtents) {}

	inline bool Intersects(const glm::vec3& point) const {
		glm::vec3 s = point - mCenter;
		return 
			(s.x <= mExtents.x && s.x >= -mExtents.x) &&
			(s.y <= mExtents.y && s.y >= -mExtents.y) &&
			(s.z <= mExtents.z && s.z >= -mExtents.z);
	}
	inline bool Intersects(const glm::vec3& point, float radius) const {
		glm::vec3 bmin = mCenter - mExtents;
		glm::vec3 bmax = mCenter + mExtents;
		float sqDist = 0.0f;
		for (int i = 0; i < 3; i++) {
			if (point[i] < bmin[i]) sqDist += (bmin[i] - point[i]) * (bmin[i] - point[i]);
			if (point[i] > bmax[i]) sqDist += (point[i] - bmax[i]) * (point[i] - bmax[i]);
		}

		return sqDist <= radius * radius;
	}
	inline bool Intersects(const AABB& box) const {
		glm::vec3 MinA = mCenter - mExtents;
		glm::vec3 MaxA = mCenter + mExtents;

		glm::vec3 MinB = box.mCenter - box.mExtents;
		glm::vec3 MaxB = box.mCenter + box.mExtents;

		// for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then return false
		bool dx = (MinA.x > MaxB.x) || (MinB.x > MaxA.x);
		bool dy = (MinA.y > MaxB.y) || (MinB.y > MaxA.y);
		bool dz = (MinA.z > MaxB.z) || (MinB.z > MaxA.z);

		return !(dx || dy || dz);
	}
};

struct Bounds {
	glm::vec3 mCenter;
	glm::vec3 mExtents;
	glm::quat mOrientation;

	Bounds(const glm::vec3& center, const glm::vec3& extents) : mCenter(center), mExtents(extents), mOrientation(glm::quat(1.f, 0.f, 0.f, 0.f)) {}
	Bounds(const glm::vec3& center, const glm::vec3& extents, const glm::quat& orientation) : mCenter(center), mExtents(extents), mOrientation(orientation) {}

	inline bool Intersects(const glm::vec3& point) const {
		glm::vec3 s = inverse(mOrientation) * (point - mCenter);
		return
			(s.x <= mExtents.x && s.x >= -mExtents.x) &&
			(s.y <= mExtents.y && s.y >= -mExtents.y) &&
			(s.z <= mExtents.z && s.z >= -mExtents.z);
	}

	inline bool Intersects(const glm::vec3& point, float radius) const {
		glm::vec3 s = inverse(mOrientation) * (point - mCenter);
		float sqDist = 0.0f;
		for (int i = 0; i < 3; i++) {
			if (s[i] < -mExtents[i]) sqDist += (-mExtents[i] - s[i]) * (-mExtents[i] - s[i]);
			if (s[i] > mExtents[i]) sqDist += (s[i] - mExtents[i]) * (s[i] - mExtents[i]);
		}

		return sqDist <= radius * radius;
	}
};

#endif