#pragma once

#include <glm/glm.hpp>

// Tests whether a ray intersects an Axis-Aligned Bounding Box (AABB)
//
// rayOrigin   : world-space start of the ray (camera position)
// rayDir      : normalized world-space ray direction
// aabbMin     : minimum corner of the AABB (world-space)
// aabbMax     : maximum corner of the AABB (world-space)
// outDistance : distance from ray origin to first intersection point
//
// Returns true if the ray hits the box, false otherwise.
bool RayIntersectsAABB(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDir,
    const glm::vec3& aabbMin,
    const glm::vec3& aabbMax,
    float& outDistance
);