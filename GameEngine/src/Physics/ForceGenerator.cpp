#include "../include/Physics/ForceGenerator.h"
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>

uint64_t ForceGenerator::nextID = 1;


// Base
ForceGenerator::ForceGenerator(const std::string& name,
    ForceGeneratorType type,
    const glm::vec3& position,
    float radius,
    float strength)
    : name(name),
    type(type),
    position(position),
    radius(radius),
    enabled(true),
    strength(strength),
    id(nextID++)
{
}

bool ForceGenerator::isInRange(const glm::vec3& bodyPos) const
{
    // radius == 0 means infinite range
    if (radius <= 0.0f) return true;
    return glm::distance(bodyPos, position) <= radius;
}

// WIND
WindGenerator::WindGenerator(const std::string& name,
    const glm::vec3& position,
    float radius,
    const glm::vec3& direction,
    float strength)
    : ForceGenerator(name, ForceGeneratorType::WIND, position, radius, strength)
{
    setDirection(direction);
}

void WindGenerator::setDirection(const glm::vec3& dir)
{
    float len = glm::length(dir);
    direction = (len > 0.0001f) ? dir / len : glm::vec3(1, 0, 0);
}

void WindGenerator::apply(btRigidBody* body, const glm::vec3& bodyPos, float deltaTime)
{
    if (!body || !isInRange(bodyPos)) return;

    // Constant force regardless of distance
    btVector3 force(
        direction.x * strength,
        direction.y * strength,
        direction.z * strength
    );

    body->activate(true);
    body->applyCentralForce(force);
}

// GRAVITY WELL
GravityWellGenerator::GravityWellGenerator(const std::string& name,
    const glm::vec3& position,
    float radius,
    float strength,
    float minDistance)
    : ForceGenerator(name, ForceGeneratorType::GRAVITY_WELL, position, radius, strength),
    minDistance(minDistance)
{
}

void GravityWellGenerator::apply(btRigidBody* body, const glm::vec3& bodyPos, float deltaTime)
{
    if (!body || !isInRange(bodyPos)) return;

    glm::vec3 delta = position - bodyPos;
    float dist = glm::length(delta);

    // Clamp to avoid infinite force at the center
    dist = std::max(dist, minDistance);

    // Direction toward center (positive strength = attract, negative = repel)
    glm::vec3 dir = delta / dist;

    // Inverse-square falloff: force = strength / dist^2
    float forceMag = strength / (dist * dist);

    btVector3 force(
        dir.x * forceMag,
        dir.y * forceMag,
        dir.z * forceMag
    );

    body->activate(true);
    body->applyCentralForce(force);
}

// VORTEX
VortexGenerator::VortexGenerator(const std::string& name,
    const glm::vec3& position,
    float radius,
    const glm::vec3& axis,
    float rotationStrength,
    float pullStrength)
    : ForceGenerator(name, ForceGeneratorType::VORTEX, position, radius, rotationStrength),
    pullStrength(pullStrength)
{
    float len = glm::length(axis);
    this->axis = (len > 0.0001f) ? axis / len : glm::vec3(0, 1, 0);
}

void VortexGenerator::apply(btRigidBody* body, const glm::vec3& bodyPos, float deltaTime)
{
    if (!body || !isInRange(bodyPos)) return;

    glm::vec3 toCenter = position - bodyPos;
    float dist = glm::length(toCenter);

    if (dist < 0.001f) return;  // at the exact center, skip

    // Tangential force: perpendicular to both the axis and the direction to center
    // This creates the spinning/orbital motion
    glm::vec3 tangent = glm::normalize(glm::cross(axis, toCenter));
    glm::vec3 rotForce = tangent * strength;

    // Inward pull toward the center axis
    glm::vec3 inward = glm::normalize(toCenter);
    glm::vec3 pullForce = inward * pullStrength;

    btVector3 totalForce(
        rotForce.x + pullForce.x,
        rotForce.y + pullForce.y,
        rotForce.z + pullForce.z
    );

    body->activate(true);
    body->applyCentralForce(totalForce);
}

// EXPLOSION
ExplosionGenerator::ExplosionGenerator(const std::string& name,
    const glm::vec3& position,
    float radius,
    float strength)
    : ForceGenerator(name, ForceGeneratorType::EXPLOSION, position, radius, strength),
    fired(false)
{
}

void ExplosionGenerator::apply(btRigidBody* body, const glm::vec3& bodyPos, float deltaTime)
{
    // Explosion only fires once across all bodies in the same frame
    // fired is set to true by the registry after all bodies have been processed
    if (!body || !isInRange(bodyPos)) return;

    glm::vec3 dir = bodyPos - position;
    float dist = glm::length(dir);

    if (dist < 0.001f)
    {
        // Body is at the epicenter — apply upward force
        dir = glm::vec3(0, 1, 0);
        dist = 1.0f;
    }

    // Linear falloff: full force at center, zero at edge of radius
    float falloff = 1.0f - (dist / radius);
    falloff = std::max(falloff, 0.0f);

    glm::vec3 normalizedDir = dir / dist;
    float impulseMag = strength * falloff;

    btVector3 impulse(
        normalizedDir.x * impulseMag,
        normalizedDir.y * impulseMag,
        normalizedDir.z * impulseMag
    );

    body->activate(true);
    body->applyCentralImpulse(impulse);

    // Mark fired so isExpired() returns true and the registry removes this generator
    fired = true;
}