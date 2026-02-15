#pragma once

#include <functional>
#include <memory>
#include <glm/glm.hpp>
#include "../Physics/ConstraintParams.h"

class GameObject;
class Constraint;

// Commands for creating and managing constraints from debug UI
struct ConstraintDebugCommands
{
    // ===== Creation Commands =====

    // Create basic constraints between two objects
    std::function<Constraint* (GameObject* objA, GameObject* objB)> createFixed;

    std::function<Constraint* (GameObject* objA, GameObject* objB,
        const glm::vec3& worldPivot, const glm::vec3& worldAxis)> createHinge;

    std::function<Constraint* (GameObject* objA, GameObject* objB,
        const HingeParams& params)> createHingeAdvanced;

    std::function<Constraint* (GameObject* objA, GameObject* objB,
        const SliderParams& params)> createSlider;

    std::function<Constraint* (GameObject* objA, GameObject* objB,
        float stiffness, float damping)> createSpring;

    std::function<Constraint* (GameObject* objA, GameObject* objB,
        const SpringParams& params)> createSpringAdvanced;

    std::function<Constraint* (GameObject* objA, GameObject* objB,
        const Generic6DofParams& params)> createGeneric6Dof;

    // ===== Management Commands =====

    std::function<void(Constraint* constraint)> removeConstraint;
    std::function<bool(const std::string& name)> removeConstraintByName;
    std::function<void(GameObject* obj)> removeConstraintsForObject;
    std::function<void()> clearAllConstraints;

    // ===== Query Commands =====

    std::function<Constraint* (const std::string& name)> findConstraintByName;
    std::function<std::vector<Constraint*>(GameObject* obj)> findConstraintsForObject;
    std::function<std::vector<Constraint*>(ConstraintType type)> findConstraintsByType;
};