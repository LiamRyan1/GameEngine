#ifndef CONSTRAINTPARAMS_H
#define CONSTRAINTPARAMS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

enum class ConstraintType {
    FIXED,
    HINGE,
    SLIDER,
    SPRING,
    GENERIC_6DOF
};

struct HingeParams {
    glm::vec3 pivotA{ 0, 0, 0 };
    glm::vec3 pivotB{ 0, 0, 0 };
    glm::vec3 axisA{ 0, 1, 0 };
    glm::vec3 axisB{ 0, 1, 0 };

    bool useLimits = false;
    float lowerLimit = 0.0f;
    float upperLimit = 0.0f;

    bool useMotor = false;
    float motorTargetVelocity = 0.0f;
    float motorMaxImpulse = 0.0f;
};

struct SliderParams {
    glm::vec3 frameAPos{ 0, 0, 0 };
    glm::quat frameARot{ 1, 0, 0, 0 };
    glm::vec3 frameBPos{ 0, 0, 0 };
    glm::quat frameBRot{ 1, 0, 0, 0 };

    bool useLimits = false;
    float lowerLimit = 0.0f;
    float upperLimit = 0.0f;

    bool useMotor = false;
    float motorTargetVelocity = 0.0f;
    float motorMaxForce = 0.0f;
};

struct SpringParams {
    glm::vec3 pivotA{ 0, 0, 0 };
    glm::vec3 pivotB{ 0, 0, 0 };
    glm::quat rotA{ 1, 0, 0, 0 };
    glm::quat rotB{ 1, 0, 0, 0 };

    // Spring axes: 0-2 = linear X,Y,Z; 3-5 = angular X,Y,Z
    bool enableSpring[6] = { false, false, false, false, false, false };
    float stiffness[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    float damping[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
};
struct Generic6DofParams {
    glm::vec3 pivotA{ 0, 0, 0 };
    glm::vec3 pivotB{ 0, 0, 0 };
    glm::quat rotA{ 1, 0, 0, 0 };
    glm::quat rotB{ 1, 0, 0, 0 };

    bool useLinearLimits[3] = { false, false, false };
    float lowerLinearLimit[3] = { 0.0f, 0.0f, 0.0f };
    float upperLinearLimit[3] = { 0.0f, 0.0f, 0.0f };

    bool useAngularLimits[3] = { false, false, false };
    float lowerAngularLimit[3] = { 0.0f, 0.0f, 0.0f };
    float upperAngularLimit[3] = { 0.0f, 0.0f, 0.0f };
};

#endif // CONSTRAINTPARAMS_H