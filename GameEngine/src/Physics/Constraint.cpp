#include "../include/Physics/Constraint.h"
#include "../include/Scene/GameObject.h"
#include <iostream>
#include <cmath>

Constraint::Constraint(btTypedConstraint* bulletConstraint,
    ConstraintType type,
    GameObject* objA,
    GameObject* objB)
    : constraint(bulletConstraint),
    type(type),
    bodyA(objA),
    bodyB(objB),
    breakable(false),
    breakForce(INFINITY),
    breakTorque(INFINITY)
{
    if (constraint) {
        constraint->setUserConstraintPtr(this);
    }
}

Constraint::~Constraint() {
    if (constraint) {
        delete constraint;
        constraint = nullptr;
    }
}

// Getters

bool Constraint::isBroken() const {
    if (!constraint) return true;
    return !constraint->isEnabled();
}

// Setters

void Constraint::setEnabled(bool enabled) {
    if (constraint) {
        constraint->setEnabled(enabled);
    }
}

void Constraint::setBreakingThreshold(float force, float torque) {
    breakable = true;
    breakForce = force;
    breakTorque = torque;

    if (constraint) {
        constraint->setBreakingImpulseThreshold(force);
    }

    std::cout << "Set breaking threshold: force=" << force
        << ", torque=" << torque << std::endl;
}

// ========== Type-Specific Controls (Hinge) ==========

void Constraint::setAngleLimits(float lower, float upper) {
    if (type != ConstraintType::HINGE) {
        std::cerr << "Warning: setAngleLimits() only works for HINGE constraints" << std::endl;
        return;
    }

    btHingeConstraint* hinge = static_cast<btHingeConstraint*>(constraint);
    if (hinge) {
        hinge->setLimit(lower, upper);
        std::cout << "Set hinge limits: [" << lower << ", " << upper << "]" << std::endl;
    }
}

void Constraint::enableMotor(float targetVelocity, float maxImpulse) {
    if (type != ConstraintType::HINGE) {
        std::cerr << "Warning: enableMotor() only works for HINGE constraints" << std::endl;
        return;
    }

    btHingeConstraint* hinge = static_cast<btHingeConstraint*>(constraint);
    if (hinge) {
        hinge->enableMotor(true);
        hinge->setMotorTarget(targetVelocity, 1.0f);
        hinge->setMaxMotorImpulse(maxImpulse);
        std::cout << "Enabled hinge motor: velocity=" << targetVelocity
            << ", maxImpulse=" << maxImpulse << std::endl;
    }
}

void Constraint::disableMotor() {
    if (type != ConstraintType::HINGE) {
        std::cerr << "Warning: disableMotor() only works for HINGE constraints" << std::endl;
        return;
    }

    btHingeConstraint* hinge = static_cast<btHingeConstraint*>(constraint);
    if (hinge) {
        hinge->enableMotor(false);
        std::cout << "Disabled hinge motor" << std::endl;
    }
}

float Constraint::getHingeAngle() const {
    if (type != ConstraintType::HINGE) {
        std::cerr << "Warning: getHingeAngle() only works for HINGE constraints" << std::endl;
        return 0.0f;
    }

    btHingeConstraint* hinge = static_cast<btHingeConstraint*>(constraint);
    if (hinge) {
        return hinge->getHingeAngle();
    }
    return 0.0f;
}

// ========== Type-Specific Controls (Slider) ==========

void Constraint::setLinearLimits(float lower, float upper) {
    if (type != ConstraintType::SLIDER) {
        std::cerr << "Warning: setLinearLimits() only works for SLIDER constraints" << std::endl;
        return;
    }

    btSliderConstraint* slider = static_cast<btSliderConstraint*>(constraint);
    if (slider) {
        slider->setLowerLinLimit(lower);
        slider->setUpperLinLimit(upper);
        std::cout << "Set slider limits: [" << lower << ", " << upper << "]" << std::endl;
    }
}

void Constraint::enableLinearMotor(float targetVelocity, float maxForce) {
    if (type != ConstraintType::SLIDER) {
        std::cerr << "Warning: enableLinearMotor() only works for SLIDER constraints" << std::endl;
        return;
    }

    btSliderConstraint* slider = static_cast<btSliderConstraint*>(constraint);
    if (slider) {
        slider->setPoweredLinMotor(true);
        slider->setTargetLinMotorVelocity(targetVelocity);
        slider->setMaxLinMotorForce(maxForce);
        std::cout << "Enabled slider motor: velocity=" << targetVelocity
            << ", maxForce=" << maxForce << std::endl;
    }
}

float Constraint::getSliderPosition() const {
    if (type != ConstraintType::SLIDER) {
        std::cerr << "Warning: getSliderPosition() only works for SLIDER constraints" << std::endl;
        return 0.0f;
    }

    btSliderConstraint* slider = static_cast<btSliderConstraint*>(constraint);
    if (slider) {
        return slider->getLinearPos();
    }
    return 0.0f;
}

// ========== Type-Specific Controls (Spring) ==========

void Constraint::setSpringStiffness(int axis, float stiffness) {
    if (type != ConstraintType::SPRING) {
        std::cerr << "Warning: setSpringStiffness() only works for SPRING constraints" << std::endl;
        return;
    }

    if (axis < 0 || axis >= 6) {
        std::cerr << "Error: Invalid axis " << axis << " (must be 0-5)" << std::endl;
        return;
    }

    btGeneric6DofSpringConstraint* spring = static_cast<btGeneric6DofSpringConstraint*>(constraint);
    if (spring) {
        spring->setStiffness(axis, stiffness);
        std::cout << "Set spring stiffness on axis " << axis << ": " << stiffness << std::endl;
    }
}

void Constraint::setSpringDamping(int axis, float damping) {
    if (type != ConstraintType::SPRING) {
        std::cerr << "Warning: setSpringDamping() only works for SPRING constraints" << std::endl;
        return;
    }

    if (axis < 0 || axis >= 6) {
        std::cerr << "Error: Invalid axis " << axis << " (must be 0-5)" << std::endl;
        return;
    }

    btGeneric6DofSpringConstraint* spring = static_cast<btGeneric6DofSpringConstraint*>(constraint);
    if (spring) {
        spring->setDamping(axis, damping);
        std::cout << "Set spring damping on axis " << axis << ": " << damping << std::endl;
    }
}

// ========== Debug ==========

void Constraint::printInfo() const {
    std::cout << "=== Constraint Info ===" << std::endl;
    std::cout << "Name: " << (name.empty() ? "(unnamed)" : name) << std::endl;
    std::cout << "Type: ";

    switch (type) {
    case ConstraintType::FIXED: std::cout << "FIXED"; break;
    case ConstraintType::HINGE: std::cout << "HINGE"; break;
    case ConstraintType::SLIDER: std::cout << "SLIDER"; break;
    case ConstraintType::SPRING: std::cout << "SPRING"; break;
    case ConstraintType::GENERIC_6DOF: std::cout << "GENERIC_6DOF"; break;
    }
    std::cout << std::endl;

    std::cout << "Enabled: " << (constraint && constraint->isEnabled() ? "Yes" : "No") << std::endl;
    std::cout << "Breakable: " << (breakable ? "Yes" : "No") << std::endl;

    if (breakable) {
        std::cout << "Break Force: " << breakForce << std::endl;
        std::cout << "Break Torque: " << breakTorque << std::endl;
    }

    // Type-specific info
    if (type == ConstraintType::HINGE && constraint) {
        btHingeConstraint* hinge = static_cast<btHingeConstraint*>(constraint);
        std::cout << "Current Angle: " << hinge->getHingeAngle() << " radians" << std::endl;
    }
    else if (type == ConstraintType::SLIDER && constraint) {
        btSliderConstraint* slider = static_cast<btSliderConstraint*>(constraint);
        std::cout << "Current Position: " << slider->getLinearPos() << std::endl;
    }

    std::cout << "======================" << std::endl;
}