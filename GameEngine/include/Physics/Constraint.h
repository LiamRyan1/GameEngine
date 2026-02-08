#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "ConstraintParams.h"
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

class GameObject;

class Constraint {
private:
    btTypedConstraint* constraint; 
    ConstraintType type;
    GameObject* bodyA;
    GameObject* bodyB;
    std::string name;

    bool breakable;
    float breakForce;
    float breakTorque;

public:
    Constraint(btTypedConstraint* bulletConstraint,
        ConstraintType type,
        GameObject* objA,
        GameObject* objB = nullptr);

    ~Constraint();

    Constraint(const Constraint&) = delete;
    Constraint& operator=(const Constraint&) = delete;

    // Getters
    btTypedConstraint* getBulletConstraint() const { return constraint; }
    ConstraintType getType() const { return type; }
    GameObject* getBodyA() const { return bodyA; }
    GameObject* getBodyB() const { return bodyB; }
    const std::string& getName() const { return name; }
    bool isBreakable() const { return breakable; }
    float getBreakForce() const { return breakForce; }
    float getBreakTorque() const { return breakTorque; }
    bool isBroken() const;

    // Setters
    void setName(const std::string& newName) { name = newName; }
    void setEnabled(bool enabled);
    void setBreakingThreshold(float force, float torque);

    // Hinge controls
    void setAngleLimits(float lower, float upper);
    void enableMotor(float targetVelocity, float maxImpulse);
    void disableMotor();
    float getHingeAngle() const;

    // Slider controls
    void setLinearLimits(float lower, float upper);
    void enableLinearMotor(float targetVelocity, float maxForce);
    float getSliderPosition() const;

    // Spring controls
    void setSpringStiffness(int axis, float stiffness);
    void setSpringDamping(int axis, float damping);

    // Debug
    void printInfo() const;
};

// Helper conversion functions
inline btVector3 toBullet(const glm::vec3& v) {
    return btVector3(v.x, v.y, v.z);
}

inline btQuaternion toBullet(const glm::quat& q) {
    return btQuaternion(q.x, q.y, q.z, q.w);
}

inline glm::vec3 fromBullet(const btVector3& v) {
    return glm::vec3(v.x(), v.y(), v.z());
}

inline glm::quat fromBullet(const btQuaternion& q) {
    return glm::quat(q.w(), q.x(), q.y(), q.z());
}

#endif // CONSTRAINT_H