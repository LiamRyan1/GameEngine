#ifndef FORCE_GENERATOR_H
#define FORCE_GENERATOR_H

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>

class GameObject;

/**
 * @brief Types of area-based force generators.
 *
 * WIND        — Constant directional force applied to all bodies in a box region.
 * GRAVITY_WELL — Pulls bodies toward a point, strength falls off with distance.
 * VORTEX      — Rotational force around a central axis.
 * EXPLOSION   — One-shot radial burst outward from a point. Auto-expires.
 */
enum class ForceGeneratorType {
    WIND,
    GRAVITY_WELL,
    VORTEX,
    EXPLOSION
};


// Base class
// All generators share common properties like position, radius, strength, and enabled state.
// Each generator applies its force in a different way based on its type and parameters.
// The registry will call apply() on each generator for every body in the scene each frame.
// The generator itself is responsible for checking if the body is in range and applying the appropriate force.
class ForceGenerator {
protected:
    std::string          name;
    ForceGeneratorType   type;
    glm::vec3            position;   // World-space center of effect
    float                radius;     // Radius of effect (0 = infinite / whole world)
    bool                 enabled;
    float                strength;

    uint64_t id;
    static uint64_t nextID;

public:
    ForceGenerator(const std::string& name,
        ForceGeneratorType type,
        const glm::vec3& position,
        float radius,
        float strength);

    virtual ~ForceGenerator() = default;

	// apply force to single rigid body based on its position and the generator's parameters
    virtual void apply(btRigidBody* body, const glm::vec3& bodyPos, float deltaTime) = 0;

	// removes one -shot generators after they have fired (e.g. Explosion)
    virtual bool isExpired() const { return false; }

    // Getters
    uint64_t              getID()       const { return id; }
    const std::string& getName()     const { return name; }
    ForceGeneratorType    getType()     const { return type; }
    glm::vec3             getPosition() const { return position; }
    float                 getRadius()   const { return radius; }
    float                 getStrength() const { return strength; }
    bool                  isEnabled()   const { return enabled; }

    // Setters
    void setName(const std::string& n) { name = n; }
    void setPosition(const glm::vec3& p) { position = p; }
    void setRadius(float r) { radius = r; }
    void setStrength(float s) { strength = s; }
    void setEnabled(bool e) { enabled = e; }

protected:
    /** Returns true if bodyPos is within this generator's radius of effect. */
    bool isInRange(const glm::vec3& bodyPos) const;
};


// WIND
// applies constant directional force to all bodies in a box region. Force does not fall off with distance.
class WindGenerator : public ForceGenerator {
private:
    glm::vec3 direction;  // Normalized wind direction

public:
    WindGenerator(const std::string& name,
        const glm::vec3& position,
        float radius,
        const glm::vec3& direction,
        float strength);

    void apply(btRigidBody* body, const glm::vec3& bodyPos, float deltaTime) override;

    void      setDirection(const glm::vec3& dir);
    glm::vec3 getDirection() const { return direction; }
};

// GRAVITY WELL
// Pulls bodies toward a point, strength falls off with distance. Force is always directed toward the center.
class GravityWellGenerator : public ForceGenerator {
private:
    float minDistance;  // Clamps distance to avoid infinite force at center

public:
    GravityWellGenerator(const std::string& name,
        const glm::vec3& position,
        float radius,
        float strength,
        float minDistance = 1.0f);

    void apply(btRigidBody* body, const glm::vec3& bodyPos, float deltaTime) override;

    void  setMinDistance(float d) { minDistance = d; }
    float getMinDistance() const { return minDistance; }
};

// VORTEX
// applies rotational force around a central axis, creating a swirling motion. Also includes an inward pull toward the center axis to keep bodies orbiting rather than flying outward.
class VortexGenerator : public ForceGenerator {
private:
    glm::vec3 axis;           // Normalized rotation axis (usually up)
    float     pullStrength;   // How strongly bodies are pulled toward center

public:
    VortexGenerator(const std::string& name,
        const glm::vec3& position,
        float radius,
        const glm::vec3& axis,
        float rotationStrength,
        float pullStrength);

    void apply(btRigidBody* body, const glm::vec3& bodyPos, float deltaTime) override;

    void      setAxis(const glm::vec3& a) { axis = glm::normalize(a); }
    void      setPullStrength(float s) { pullStrength = s; }
    glm::vec3 getAxis()        const { return axis; }
    float     getPullStrength()const { return pullStrength; }
};

// EXPLOSION
// One-shot radial burst outward from a point. Force is applied only once, then the generator expires and is removed from the registry. Strength falls off with distance.
class ExplosionGenerator : public ForceGenerator {
private:
    bool fired;  // True after the explosion has been applied once

public:
    ExplosionGenerator(const std::string& name,
        const glm::vec3& position,
        float radius,
        float strength);

    void apply(btRigidBody* body, const glm::vec3& bodyPos, float deltaTime) override;
    bool isExpired() const override { return fired; }

    /** Called by the registry after all bodies have been processed this frame. */
    void markFired() { fired = true; }
};

#endif // FORCE_GENERATOR_H