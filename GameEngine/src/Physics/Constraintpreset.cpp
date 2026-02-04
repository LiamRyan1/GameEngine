#include "../include/GameObject.h"
#include "../include/ConstraintPreset.h"
#include <iostream>
#include <glm/gtc/constants.hpp>

//  FIXED Constraints 

std::unique_ptr<Constraint> ConstraintPreset::createFixed(
    GameObject* objA, GameObject* objB)
{
    if (!objA || !objA->hasPhysics()) {
        std::cerr << "Error: Cannot create fixed constraint - objA missing or no physics" << std::endl;
        return nullptr;
    }

    if (!objB || !objB->hasPhysics()) {
        std::cerr << "Error: Cannot create fixed constraint - objB missing or no physics" << std::endl;
        return nullptr;
    }

    btRigidBody* rbA = objA->getRigidBody();
    btRigidBody* rbB = objB->getRigidBody();

    // Calculate relative transform from A to B
    btTransform worldA = rbA->getWorldTransform();
    btTransform worldB = rbB->getWorldTransform();
    btTransform frameInA = worldA.inverse() * worldB;// B's position relative to A
    btTransform frameInB;
    frameInB.setIdentity();

    // Create bullet fixed constraint (generic 6DOF with all DOFs locked)
    btGeneric6DofConstraint* fixedConstraint = new btGeneric6DofConstraint(
        *rbA, *rbB, frameInA, frameInB, true
    );

    // Lock all axes
    fixedConstraint->setLinearLowerLimit(btVector3(0, 0, 0));
    fixedConstraint->setLinearUpperLimit(btVector3(0, 0, 0));
    fixedConstraint->setAngularLowerLimit(btVector3(0, 0, 0));
    fixedConstraint->setAngularUpperLimit(btVector3(0, 0, 0));

	//wrap bullet in constraint class
    auto constraint = std::make_unique<Constraint>(
        fixedConstraint, ConstraintType::FIXED, objA, objB
    );
	//return ownship of pointer
    std::cout << "Created FIXED constraint" << std::endl;
    return constraint;
}

// HINGE Constraints 

std::unique_ptr<Constraint> ConstraintPreset::createHinge(
    GameObject* objA, GameObject* objB, const HingeParams& params)
{
    if (!objA || !objA->hasPhysics()) {
        std::cerr << "Error: Cannot create hinge - objA missing or no physics" << std::endl;
        return nullptr;
    }
	// get physics bodies if they exist, hinge can be created with just one body (hinged to world)
    btRigidBody* rbA = objA->getRigidBody();
    btRigidBody* rbB = objB ? objB->getRigidBody() : nullptr;

	// store as bullet hinge constraint pointer
    btHingeConstraint* hinge;

	// if we have two bodies, create hinge between them, otherwise hinge to world
    if (rbB) {
        hinge = new btHingeConstraint(
            *rbA, *rbB,
            toBullet(params.pivotA), toBullet(params.pivotB),
            toBullet(params.axisA), toBullet(params.axisB)
        );
    }
    else {
        hinge = new btHingeConstraint(
            *rbA,
            toBullet(params.pivotA),
            toBullet(params.axisA)
        );
    }

    // Apply limits if enabled
    if (params.useLimits) {
        hinge->setLimit(params.lowerLimit, params.upperLimit);
    }

    // Apply motor if enabled
    if (params.useMotor) {
        hinge->enableMotor(true);
        hinge->setMotorTarget(params.motorTargetVelocity, 1.0f);
        hinge->setMaxMotorImpulse(params.motorMaxImpulse);
    }

    auto constraint = std::make_unique<Constraint>(
        hinge, ConstraintType::HINGE, objA, objB
    );

    std::cout << "Created HINGE constraint";
    if (params.useLimits) {
        std::cout << " with limits [" << params.lowerLimit << ", " << params.upperLimit << "]";
    }
    std::cout << std::endl;

    return constraint;
}

std::unique_ptr<Constraint> ConstraintPreset::createHinge(
    GameObject* objA, GameObject* objB,
    const glm::vec3& worldPivot, const glm::vec3& worldAxis)
{
    if (!objA || !objA->hasPhysics()) {
        std::cerr << "Error: Cannot create hinge - objA missing or no physics" << std::endl;
        return nullptr;
    }

    // Convert world-space pivot to local space
    HingeParams params;
    params.pivotA = worldPivot - objA->getPosition();
    params.axisA = glm::normalize(worldAxis);

    if (objB && objB->hasPhysics()) {
        params.pivotB = worldPivot - objB->getPosition();
        params.axisB = glm::normalize(worldAxis);
    }
    else {
        params.pivotB = worldPivot;
        params.axisB = glm::normalize(worldAxis);
    }

    return createHinge(objA, objB, params);
}

//  SLIDER Constraints 

std::unique_ptr<Constraint> ConstraintPreset::createSlider(
    GameObject* objA, GameObject* objB, const SliderParams& params)
{
    if (!objA || !objA->hasPhysics()) {
        std::cerr << "Error: Cannot create slider - objA missing or no physics" << std::endl;
        return nullptr;
    }

    if (!objB || !objB->hasPhysics()) {
        std::cerr << "Error: Cannot create slider - objB missing or no physics" << std::endl;
        return nullptr;
    }

    btRigidBody* rbA = objA->getRigidBody();
    btRigidBody* rbB = objB->getRigidBody();

    // Create transforms for the slider frames
	// frameInA is the position and orientation of the slider in A's local space, if its attached to A at (0,0) at a 25% rotation around z ect
    btTransform frameInA, frameInB;
	frameInA.setOrigin(toBullet(params.frameAPos)); // set the position of the frame in A's local space i.e . where the slider is attached to A
    frameInA.setRotation(toBullet(params.frameARot));
    frameInB.setOrigin(toBullet(params.frameBPos));
    frameInB.setRotation(toBullet(params.frameBRot));

    btSliderConstraint* slider = new btSliderConstraint(
        *rbA, *rbB, frameInA, frameInB, true
    );

    // Apply limits if enabled
    if (params.useLimits) {
        slider->setLowerLinLimit(params.lowerLimit);
        slider->setUpperLinLimit(params.upperLimit);
    }

    // Apply motor if enabled
    if (params.useMotor) {
        slider->setPoweredLinMotor(true);
        slider->setTargetLinMotorVelocity(params.motorTargetVelocity);
        slider->setMaxLinMotorForce(params.motorMaxForce);
    }

    auto constraint = std::make_unique<Constraint>(
        slider, ConstraintType::SLIDER, objA, objB
    );

    std::cout << "Created SLIDER constraint";
    if (params.useLimits) {
        std::cout << " with limits [" << params.lowerLimit << ", " << params.upperLimit << "]";
    }
    std::cout << std::endl;

    return constraint;
}

//  SPRING Constraints 

std::unique_ptr<Constraint> ConstraintPreset::createSpring(
    GameObject* objA, GameObject* objB, const SpringParams& params)
{
    if (!objA || !objA->hasPhysics()) {
        std::cerr << "Error: Cannot create spring - objA missing or no physics" << std::endl;
        return nullptr;
    }

    if (!objB || !objB->hasPhysics()) {
        std::cerr << "Error: Cannot create spring - objB missing or no physics" << std::endl;
        return nullptr;
    }

    btRigidBody* rbA = objA->getRigidBody();
    btRigidBody* rbB = objB->getRigidBody();

    // Create transforms
    btTransform frameInA, frameInB;
    frameInA.setOrigin(toBullet(params.pivotA));
    frameInA.setRotation(toBullet(params.rotA));
    frameInB.setOrigin(toBullet(params.pivotB));
    frameInB.setRotation(toBullet(params.rotB));

	// create bullet spring constraint between the two rigid bodies connected at the specified frames with springs enabled
    btGeneric6DofSpringConstraint* spring = new btGeneric6DofSpringConstraint(
        *rbA, *rbB, frameInA, frameInB, true
    );

    // Configure springs
    for (int i = 0; i < 6; ++i) {
        if (params.enableSpring[i]) {
            spring->enableSpring(i, true);
			spring->setStiffness(i, params.stiffness[i]); //stuffness- how resiliant to compression the spring is
			spring->setDamping(i, params.damping[i]); //damping- how much energy is lost  when the spring compresses/extends
        }
    }

    // Set equilibrium point to current position
    spring->setEquilibriumPoint();

	//wrap bullet constraint in Constraint class
    auto constraint = std::make_unique<Constraint>(
        spring, ConstraintType::SPRING, objA, objB
    );

    int activeSpringCount = 0;
    for (int i = 0; i < 6; ++i) {
        if (params.enableSpring[i]) activeSpringCount++;
    }
    std::cout << "Created SPRING constraint with " << activeSpringCount << " active axes" << std::endl;

    return constraint;
}

// Simplified spring creation with default pivots , sends to main spring creation function 
std::unique_ptr<Constraint> ConstraintPreset::createSpring(
    GameObject* objA, GameObject* objB, float stiffness, float damping)
{
    SpringParams params;

    // Enable spring on Y axis (vertical) - typical for suspension
    params.enableSpring[1] = true;
    params.stiffness[1] = stiffness;
    params.damping[1] = damping;

    // Set pivot points at object centers
    params.pivotA = glm::vec3(0.0f);
    params.pivotB = glm::vec3(0.0f);

    return createSpring(objA, objB, params);
}

// ========== GENERIC 6DOF Constraints ==========

std::unique_ptr<Constraint> ConstraintPreset::createGeneric6Dof(
    GameObject* objA, GameObject* objB, const Generic6DofParams& params)
{
    if (!objA || !objA->hasPhysics()) {
        std::cerr << "Error: Cannot create 6DOF - objA missing or no physics" << std::endl;
        return nullptr;
    }

    if (!objB || !objB->hasPhysics()) {
        std::cerr << "Error: Cannot create 6DOF - objB missing or no physics" << std::endl;
        return nullptr;
    }

    btRigidBody* rbA = objA->getRigidBody();
    btRigidBody* rbB = objB->getRigidBody();

    // Create transforms
    btTransform frameInA, frameInB;
    frameInA.setOrigin(toBullet(params.pivotA));
    frameInA.setRotation(toBullet(params.rotA));
    frameInB.setOrigin(toBullet(params.pivotB));
    frameInB.setRotation(toBullet(params.rotB));

    btGeneric6DofConstraint* dof6 = new btGeneric6DofConstraint(
        *rbA, *rbB, frameInA, frameInB, true
    );

    // Apply linear limits
    for (int i = 0; i < 3; ++i) {
        if (params.useLinearLimits[i]) {
            btVector3 lower(0, 0, 0);
            btVector3 upper(0, 0, 0);
            lower[i] = params.lowerLinearLimit[i];
            upper[i] = params.upperLinearLimit[i];
            dof6->setLinearLowerLimit(lower);
            dof6->setLinearUpperLimit(upper);
        }
    }

    // Apply angular limits
    for (int i = 0; i < 3; ++i) {
        if (params.useAngularLimits[i]) {
            btVector3 lower(0, 0, 0);
            btVector3 upper(0, 0, 0);
            lower[i] = params.lowerAngularLimit[i];
            upper[i] = params.upperAngularLimit[i];
            dof6->setAngularLowerLimit(lower);
            dof6->setAngularUpperLimit(upper);
        }
    }

    auto constraint = std::make_unique<Constraint>(
        dof6, ConstraintType::GENERIC_6DOF, objA, objB
    );

    std::cout << "Created GENERIC_6DOF constraint" << std::endl;

    return constraint;
}

// ========== Common Presets ==========

std::unique_ptr<Constraint> ConstraintPreset::createDoorHinge(
    GameObject* door, GameObject* frame, const glm::vec3& hingeWorldPos)
{
    auto hinge = createHinge(door, frame, hingeWorldPos, glm::vec3(0, 1, 0));

    if (hinge) {
        // Typical door: swings from 0 to 90 degrees
        hinge->setAngleLimits(0.0f, glm::radians(90.0f));
        hinge->setName("DoorHinge");
    }

    return hinge;
}

std::unique_ptr<Constraint> ConstraintPreset::createDrawer(
    GameObject* drawer, GameObject* cabinet, float slideDistance)
{
    SliderParams params;
    params.frameAPos = glm::vec3(0, 0, 0);
    params.frameARot = glm::quat(1, 0, 0, 0);
    params.frameBPos = glm::vec3(0, 0, 0);
    params.frameBRot = glm::quat(1, 0, 0, 0);
    params.useLimits = true;
    params.lowerLimit = 0.0f;
    params.upperLimit = slideDistance;

    auto slider = createSlider(drawer, cabinet, params);

    if (slider) {
        slider->setName("Drawer");
    }

    return slider;
}

std::unique_ptr<Constraint> ConstraintPreset::createSuspension(
    GameObject* wheel, GameObject* chassis, float stiffness, float damping)
{
    auto spring = createSpring(wheel, chassis, stiffness, damping);

    if (spring) {
        spring->setName("Suspension");
    }

    return spring;
}

std::unique_ptr<Constraint> ConstraintPreset::createRopeSegment(
    GameObject* segmentA, GameObject* segmentB, float stiffness)
{
    SpringParams params;
    params.pivotA = glm::vec3(0, -0.25f, 0);  // Bottom of segment A
    params.pivotB = glm::vec3(0, 0.25f, 0);   // Top of segment B

    // Enable springs on all linear axes for stretchy rope
    for (int i = 0; i < 3; ++i) {
        params.enableSpring[i] = true;
        params.stiffness[i] = stiffness;
        params.damping[i] = stiffness * 0.1f;  // 10% damping
    }

    auto spring = createSpring(segmentA, segmentB, params);

    if (spring) {
        spring->setName("RopeSegment");
    }

    return spring;
}

std::unique_ptr<Constraint> ConstraintPreset::createPendulum(
    GameObject* bob, GameObject* pivot, const glm::vec3& pivotWorldPos)
{
    auto hinge = createHinge(bob, pivot, pivotWorldPos, glm::vec3(0, 0, 1));

    if (hinge) {
        hinge->setName("Pendulum");
    }

    return hinge;
}