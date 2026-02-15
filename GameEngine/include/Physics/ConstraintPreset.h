#ifndef CONSTRAINTPRESET_H
#define CONSTRAINTPRESET_H

#include "Constraint.h"
#include "ConstraintParams.h"
#include <memory>
#include <glm/glm.hpp>

class GameObject;

class ConstraintPreset {
public:
    // FIXED
    static std::unique_ptr<Constraint> createFixed(GameObject* objA, GameObject* objB);

    // HINGE
    static std::unique_ptr<Constraint> createHinge(GameObject* objA, GameObject* objB, const HingeParams& params);
    static std::unique_ptr<Constraint> createHinge(GameObject* objA, GameObject* objB,
        const glm::vec3& worldPivot, const glm::vec3& worldAxis);

    // SLIDER
    static std::unique_ptr<Constraint> createSlider(GameObject* objA, GameObject* objB, const SliderParams& params);

    // SPRING
    static std::unique_ptr<Constraint> createSpring(GameObject* objA, GameObject* objB, const SpringParams& params);
    static std::unique_ptr<Constraint> createSpring(GameObject* objA, GameObject* objB, float stiffness, float damping);

    // GENERIC 6DOF
    static std::unique_ptr<Constraint> createGeneric6Dof(GameObject* objA, GameObject* objB, const Generic6DofParams& params);
};

#endif // CONSTRAINTPRESET_H