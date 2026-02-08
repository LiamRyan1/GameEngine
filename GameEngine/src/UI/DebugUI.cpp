#include "../include/Debug/DebugUI.h"
#include "../include/Rendering/DirectionalLight.h"
#include "../include/Physics/Constraint.h"
#include "../include/Physics/ConstraintRegistry.h"
#include <glm/gtc/constants.hpp>
#include "imgui.h"
#include <vector>
#include <string>
#include <iostream>
#include <cmath> // std::atan2, std::asin

static glm::vec3 QuatToEulerRad(const glm::quat& q)
{
    // Returns (pitch, yaw, roll) in radians using a standard Tait-Bryan conversion.
    // NOTE: This is good enough for editor UI. We’ll refine later if needed.
    glm::vec3 euler;

    // pitch (x)
    float sinp = 2.0f * (q.w * q.x - q.z * q.y);
    if (std::abs(sinp) >= 1.0f)
        euler.x = std::copysign(glm::half_pi<float>(), sinp);
    else
        euler.x = std::asin(sinp);

    // yaw (y)
    float siny_cosp = 2.0f * (q.w * q.y + q.x * q.z);
    float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.x * q.x);
    euler.y = std::atan2(siny_cosp, cosy_cosp);

    // roll (z)
    float sinr_cosp = 2.0f * (q.w * q.z + q.y * q.x);
    float cosr_cosp = 1.0f - 2.0f * (q.z * q.z + q.x * q.x);
    euler.z = std::atan2(sinr_cosp, cosr_cosp);

    return euler;
}

static glm::quat EulerRadToQuat(const glm::vec3& euler)
{
    // euler = (pitch, yaw, roll) in radians
    // Build quaternion from axis rotations (X then Y then Z).
    glm::quat qx = glm::angleAxis(euler.x, glm::vec3(1, 0, 0));
    glm::quat qy = glm::angleAxis(euler.y, glm::vec3(0, 1, 0));
    glm::quat qz = glm::angleAxis(euler.z, glm::vec3(0, 0, 1));
    return qy * qx * qz;
}
// ========== Helper Functions ==========

static const char* ConstraintTypeToString(ConstraintType type) {
    switch (type) {
    case ConstraintType::FIXED: return "Fixed";
    case ConstraintType::HINGE: return "Hinge";
    case ConstraintType::SLIDER: return "Slider";
    case ConstraintType::SPRING: return "Spring";
    case ConstraintType::GENERIC_6DOF: return "Generic 6DOF";
    default: return "Unknown";
    }
}

// ========== Constraint Creation Panel ==========

static void DrawConstraintCreator(const DebugUIContext& context) {
    ImGui::Begin("Constraint Creator");

    // State for object selection
    static GameObject* objectA = nullptr;
    static GameObject* objectB = nullptr;
    static int constraintTypeIndex = 0;
    static char constraintName[64] = "";

    ImGui::SeparatorText("Object Selection");

    // Display selected objects
    if (objectA) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Object A: Selected");
        if (ImGui::Button("Clear Object A")) {
            objectA = nullptr;
        }
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Object A: None");
    }

    if (objectB) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Object B: Selected");
        if (ImGui::Button("Clear Object B")) {
            objectB = nullptr;
        }
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Object B: None (Optional)");
    }

    ImGui::Separator();

    // Quick select from inspector
    if (context.selectedObject && context.selectedObject->hasPhysics()) {
        if (ImGui::Button("Set Selected as Object A")) {
            objectA = context.selectedObject;
        }
        ImGui::SameLine();
        if (ImGui::Button("Set Selected as Object B")) {
            objectB = context.selectedObject;
        }
    }
    else {
        ImGui::TextDisabled("Select a physics object in the scene");
    }

    ImGui::SeparatorText("Constraint Type");

    const char* types[] = {
        "Fixed Joint",
        "Hinge (Door/Wheel)",
        "Slider (Drawer)",
        "Spring (Suspension)",
        "Generic 6DOF"
    };
    ImGui::Combo("Type", &constraintTypeIndex, types, IM_ARRAYSIZE(types));

    // Optional name
    ImGui::InputText("Name (Optional)", constraintName, IM_ARRAYSIZE(constraintName));

    ImGui::SeparatorText("Parameters");

    // Type-specific parameters
    static float hingeAxis[3] = { 0.0f, 1.0f, 0.0f };
    static float hingePivot[3] = { 0.0f, 0.0f, 0.0f };
    static bool useHingeLimits = false;
    static float hingeLowerLimit = 0.0f;
    static float hingeUpperLimit = 90.0f;
    static bool useHingeMotor = false;
    static float hingeMotorVelocity = 1.0f;
    static float hingeMotorForce = 10.0f;

    static float sliderDistance = 2.0f;
    static bool useSliderMotor = false;
    static float sliderMotorVelocity = 1.0f;
    static float sliderMotorForce = 10.0f;

    static float springStiffness = 100.0f;
    static float springDamping = 10.0f;
    static bool springAxisEnabled[6] = { false, true, false, false, false, false }; // Y axis by default

    static bool breakable = false;
    static float breakForce = 1000.0f;
    static float breakTorque = 1000.0f;

    switch (constraintTypeIndex) {
    case 0: // Fixed
        ImGui::TextWrapped("Fixed joints lock two objects together rigidly. No parameters needed.");
        break;

    case 1: // Hinge
        ImGui::InputFloat3("Hinge Axis", hingeAxis);
        ImGui::InputFloat3("Pivot Point", hingePivot);

        ImGui::Checkbox("Use Limits", &useHingeLimits);
        if (useHingeLimits) {
            ImGui::SliderFloat("Lower Limit (deg)", &hingeLowerLimit, -180.0f, 180.0f);
            ImGui::SliderFloat("Upper Limit (deg)", &hingeUpperLimit, -180.0f, 180.0f);
        }

        ImGui::Checkbox("Use Motor", &useHingeMotor);
        if (useHingeMotor) {
            ImGui::SliderFloat("Motor Velocity", &hingeMotorVelocity, -10.0f, 10.0f);
            ImGui::SliderFloat("Motor Force", &hingeMotorForce, 0.0f, 100.0f);
        }
        break;

    case 2: // Slider
        ImGui::SliderFloat("Slide Distance", &sliderDistance, 0.1f, 10.0f);

        ImGui::Checkbox("Use Motor", &useSliderMotor);
        if (useSliderMotor) {
            ImGui::SliderFloat("Motor Velocity", &sliderMotorVelocity, -10.0f, 10.0f);
            ImGui::SliderFloat("Motor Force", &sliderMotorForce, 0.0f, 100.0f);
        }
        break;

    case 3: // Spring
        ImGui::SliderFloat("Stiffness", &springStiffness, 1.0f, 1000.0f);
        ImGui::SliderFloat("Damping", &springDamping, 0.1f, 100.0f);

        ImGui::Text("Active Axes:");
        ImGui::Checkbox("X", &springAxisEnabled[0]); ImGui::SameLine();
        ImGui::Checkbox("Y", &springAxisEnabled[1]); ImGui::SameLine();
        ImGui::Checkbox("Z", &springAxisEnabled[2]);
        break;

    case 4: // Generic 6DOF
        ImGui::TextWrapped("Advanced constraint with full control over all 6 degrees of freedom.");
        break;
    }

    ImGui::SeparatorText("Breaking");
    ImGui::Checkbox("Breakable", &breakable);
    if (breakable) {
        ImGui::SliderFloat("Break Force", &breakForce, 10.0f, 10000.0f);
        ImGui::SliderFloat("Break Torque", &breakTorque, 10.0f, 10000.0f);
    }

    ImGui::Separator();

    // Create button
    bool canCreate = objectA != nullptr && objectA->hasPhysics();
    if (!canCreate) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Need at least Object A with physics!");
    }

    ImGui::BeginDisabled(!canCreate);
    if (ImGui::Button("Create Constraint", ImVec2(-1, 0))) {
        Constraint* newConstraint = nullptr;

        switch (constraintTypeIndex) {
        case 0: // Fixed
            if (context.constraintCommands.createFixed) {
                newConstraint = context.constraintCommands.createFixed(objectA, objectB);
            }
            break;

        case 1: { // Hinge
            if (context.constraintCommands.createHingeAdvanced) {
                HingeParams params;
                params.pivotA = glm::vec3(hingePivot[0], hingePivot[1], hingePivot[2]);
                params.axisA = glm::normalize(glm::vec3(hingeAxis[0], hingeAxis[1], hingeAxis[2]));

                if (objectB) {
                    params.pivotB = glm::vec3(hingePivot[0], hingePivot[1], hingePivot[2]);
                    params.axisB = params.axisA;
                }

                params.useLimits = useHingeLimits;
                params.lowerLimit = glm::radians(hingeLowerLimit);
                params.upperLimit = glm::radians(hingeUpperLimit);
                params.useMotor = useHingeMotor;
                params.motorTargetVelocity = hingeMotorVelocity;
                params.motorMaxImpulse = hingeMotorForce;

                newConstraint = context.constraintCommands.createHingeAdvanced(objectA, objectB, params);
            }
            break;
        }

        case 2: { // Slider
            if (context.constraintCommands.createSlider) {
                SliderParams params;
                params.useLimits = true;
                params.lowerLimit = 0.0f;
                params.upperLimit = sliderDistance;
                params.useMotor = useSliderMotor;
                params.motorTargetVelocity = sliderMotorVelocity;
                params.motorMaxForce = sliderMotorForce;

                newConstraint = context.constraintCommands.createSlider(objectA, objectB, params);
            }
            break;
        }

        case 3: { // Spring
            if (context.constraintCommands.createSpringAdvanced) {
                SpringParams params;
                for (int i = 0; i < 6; i++) {
                    params.enableSpring[i] = springAxisEnabled[i];
                    params.stiffness[i] = springStiffness;
                    params.damping[i] = springDamping;
                }

                newConstraint = context.constraintCommands.createSpringAdvanced(objectA, objectB, params);
            }
            break;
        }

        case 4: { // Generic 6DOF
            if (context.constraintCommands.createGeneric6Dof) {
                Generic6DofParams params;
                newConstraint = context.constraintCommands.createGeneric6Dof(objectA, objectB, params);
            }
            break;
        }
        }

        if (newConstraint) {
            // Set name if provided
            if (strlen(constraintName) > 0) {
                newConstraint->setName(std::string(constraintName));
            }

            // Set breaking threshold if enabled
            if (breakable) {
                newConstraint->setBreakingThreshold(breakForce, breakTorque);
            }

            std::cout << "Created " << ConstraintTypeToString(newConstraint->getType()) << " constraint" << std::endl;

            // Clear selection for next constraint
            objectA = nullptr;
            objectB = nullptr;
        }
    }
    ImGui::EndDisabled();

    ImGui::End();
}

// ========== Constraint Presets Panel ==========

static void DrawConstraintPresets(const DebugUIContext& context) {
    ImGui::Begin("Constraint Presets");

    ImGui::TextWrapped("Quick create common constraint setups");
    ImGui::Separator();

    static GameObject* presetObjA = nullptr;
    static GameObject* presetObjB = nullptr;

    if (presetObjA) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Object A: Selected");
        if (ImGui::Button("Clear A")) presetObjA = nullptr;
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Object A: None");
    }

    if (presetObjB) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Object B: Selected");
        if (ImGui::Button("Clear B")) presetObjB = nullptr;
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Object B: None");
    }

    if (context.selectedObject && context.selectedObject->hasPhysics()) {
        if (ImGui::Button("Set Selected as A")) presetObjA = context.selectedObject;
        ImGui::SameLine();
        if (ImGui::Button("Set Selected as B")) presetObjB = context.selectedObject;
    }

    ImGui::Separator();

    bool canCreate = presetObjA && presetObjB && presetObjA->hasPhysics();

    ImGui::BeginDisabled(!canCreate);

    // Door Hinge
    if (ImGui::Button("Door Hinge", ImVec2(-1, 0))) {
        if (context.constraintCommands.createDoorHinge) {
            glm::vec3 hingePos = presetObjA->getPosition();
            context.constraintCommands.createDoorHinge(presetObjA, presetObjB, hingePos);
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Hinge that swings 0-90 degrees like a door");
    }

    // Drawer
    static float drawerDist = 1.5f;
    ImGui::SliderFloat("Drawer Distance", &drawerDist, 0.5f, 5.0f);
    if (ImGui::Button("Drawer Slider", ImVec2(-1, 0))) {
        if (context.constraintCommands.createDrawer) {
            context.constraintCommands.createDrawer(presetObjA, presetObjB, drawerDist);
        }
    }

    // Suspension
    static float suspStiff = 100.0f;
    static float suspDamp = 10.0f;
    ImGui::SliderFloat("Suspension Stiffness", &suspStiff, 10.0f, 500.0f);
    ImGui::SliderFloat("Suspension Damping", &suspDamp, 1.0f, 50.0f);
    if (ImGui::Button("Suspension Spring", ImVec2(-1, 0))) {
        if (context.constraintCommands.createSuspension) {
            context.constraintCommands.createSuspension(presetObjA, presetObjB, suspStiff, suspDamp);
        }
    }

    // Rope Segment
    static float ropeStiff = 50.0f;
    ImGui::SliderFloat("Rope Stiffness", &ropeStiff, 10.0f, 200.0f);
    if (ImGui::Button("Rope Segment", ImVec2(-1, 0))) {
        if (context.constraintCommands.createRopeSegment) {
            context.constraintCommands.createRopeSegment(presetObjA, presetObjB, ropeStiff);
        }
    }

    // Pendulum
    if (ImGui::Button("Pendulum", ImVec2(-1, 0))) {
        if (context.constraintCommands.createPendulum && presetObjB) {
            glm::vec3 pivotPos = presetObjB->getPosition();
            context.constraintCommands.createPendulum(presetObjA, presetObjB, pivotPos);
        }
    }

    ImGui::EndDisabled();

    if (!canCreate) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
            "Select two physics objects!");
    }

    ImGui::End();
}

// ========== Constraint List Panel ==========

static void DrawConstraintList(const DebugUIContext& context) {
    ImGui::Begin("Constraints");

   
    // Stats at top
    ImGui::Text("Total: %d", context.constraints.totalConstraints);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Active: %d", context.constraints.activeConstraints);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Broken: %d", context.constraints.brokenConstraints);

    ImGui::Separator();
    static int selectedConstraintIndex = -1;
    // Type breakdown
    if (ImGui::TreeNode("By Type")) {
        ImGui::BulletText("Fixed: %d", context.constraints.fixedCount);
        ImGui::BulletText("Hinge: %d", context.constraints.hingeCount);
        ImGui::BulletText("Slider: %d", context.constraints.sliderCount);
        ImGui::BulletText("Spring: %d", context.constraints.springCount);
        ImGui::BulletText("Generic 6DOF: %d", context.constraints.dof6Count);
        ImGui::TreePop();
    }

    ImGui::Separator();

    // Filter controls
    static int filterType = -1; // -1 = all
    const char* filterItems[] = { "All", "Fixed", "Hinge", "Slider", "Spring", "6DOF" };
    ImGui::Combo("Filter", &filterType, filterItems, IM_ARRAYSIZE(filterItems));

    static bool showBrokenOnly = false;
    ImGui::Checkbox("Broken Only", &showBrokenOnly);
   
    ImGui::Separator();

    // Clear all button
    if (ImGui::Button("Clear All Constraints", ImVec2(-1, 0))) {
        if (context.constraintCommands.clearAllConstraints) {
            context.constraintCommands.clearAllConstraints();
            selectedConstraintIndex = -1;  // Reset index
            ImGui::End();
            return;
        }
    }

    ImGui::Separator();

    ImGui::BeginChild("ConstraintList", ImVec2(0, 250), true);
    int currentIndex = 0;
    for (Constraint* constraint : context.constraints.allConstraints) {
        if (!constraint) {
            currentIndex++;
            continue;
        }

        // Apply filters
        if (showBrokenOnly && !constraint->isBroken()) {
            currentIndex++;
            continue;
        }
        if (filterType > 0) {
            ConstraintType expectedType = static_cast<ConstraintType>(filterType - 1);
            if (constraint->getType() != expectedType) {
                currentIndex++;
                continue;
            }
        }

        // Display constraint
        ImGui::PushID(currentIndex);

        bool isSelected = (selectedConstraintIndex == currentIndex);
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

        std::string label = constraint->getName().empty() ?
            ConstraintTypeToString(constraint->getType()) :
            constraint->getName();

        if (constraint->isBroken()) {
            label += " [BROKEN]";
        }

        ImGui::TreeNodeEx(label.c_str(), flags);

        if (ImGui::IsItemClicked()) {
            selectedConstraintIndex = currentIndex;
        }

        ImGui::PopID();
        currentIndex++;
    }

    ImGui::EndChild();

    // Selected constraint details
    Constraint* selectedConstraint = nullptr;
    if (selectedConstraintIndex >= 0 &&
        selectedConstraintIndex < context.constraints.allConstraints.size()) {
        selectedConstraint = context.constraints.allConstraints[selectedConstraintIndex];
    }
    if (selectedConstraint) {
        ImGui::Separator();
        ImGui::Text("Selected: %s", selectedConstraint->getName().empty() ?
            ConstraintTypeToString(selectedConstraint->getType()) :
            selectedConstraint->getName().c_str());

        if (ImGui::Button("Remove This Constraint")) {
            if (context.constraintCommands.removeConstraint) {
                context.constraintCommands.removeConstraint(selectedConstraint);
                selectedConstraintIndex = -1;
            }
        }
    }

    ImGui::End();
}

// ========== Constraint Inspector Panel ==========
static void DrawConstraintInspector(const DebugUIContext& context) {

    ImGui::Begin("Constraint Inspector");

    // Show constraints for selected object
    if (!context.selectedObject) {
        ImGui::Text("No object selected");
        ImGui::Text("Select an object to see its constraints");
        ImGui::End();
        return;
    }

    ImGui::Text("Object Selected");
    ImGui::Separator();

    // Find constraints for this object
    std::vector<Constraint*> objectConstraints;
    if (context.constraintCommands.findConstraintsForObject) {
        objectConstraints = context.constraintCommands.findConstraintsForObject(context.selectedObject);
    }

   
    ImGui::Text("Constraints: %d", (int)objectConstraints.size());

    if (objectConstraints.empty()) {
        ImGui::TextDisabled("This object has no constraints");

        if (ImGui::Button("Remove All Constraints")) {
            if (context.constraintCommands.removeConstraintsForObject) {
                context.constraintCommands.removeConstraintsForObject(context.selectedObject);
            }
        }
    }
    else {
        ImGui::Separator();

        // List each constraint with controls
        static int selectedConstraintIndex = -1;

        if (selectedConstraintIndex >= objectConstraints.size()) {
            selectedConstraintIndex = -1;
        }
        for (int i = 0; i < objectConstraints.size(); i++) {
            Constraint* constraint = objectConstraints[i];
            ImGui::PushID(i);

            bool isSelected = (selectedConstraintIndex == i);
            if (ImGui::Selectable(
                constraint->getName().empty() ?
                ConstraintTypeToString(constraint->getType()) :
                constraint->getName().c_str(),
                isSelected)) {
                selectedConstraintIndex = i;
            }

            ImGui::PopID();
        }

        // Show details for selected constraint
        if (selectedConstraintIndex >= 0 && selectedConstraintIndex < objectConstraints.size()) {
            Constraint* constraint = objectConstraints[selectedConstraintIndex];

            ImGui::Separator();
            ImGui::Text("Type: %s", ConstraintTypeToString(constraint->getType()));
            ImGui::Text("Enabled: %s", constraint->isBroken() ? "No (Broken)" : "Yes");
            ImGui::Text("Breakable: %s", constraint->isBreakable() ? "Yes" : "No");

            if (constraint->isBreakable()) {
                ImGui::Text("Break Force: %.1f", constraint->getBreakForce());
                ImGui::Text("Break Torque: %.1f", constraint->getBreakTorque());
            }

            ImGui::Separator();

            // Type-specific controls
            switch (constraint->getType()) {
            case ConstraintType::HINGE: {
                float currentAngle = constraint->getHingeAngle();
                ImGui::Text("Current Angle: %.2f deg", glm::degrees(currentAngle));

                static float newLower = 0.0f;
                static float newUpper = 90.0f;
                ImGui::SliderFloat("Lower Limit", &newLower, -180.0f, 180.0f);
                ImGui::SliderFloat("Upper Limit", &newUpper, -180.0f, 180.0f);
                if (ImGui::Button("Apply Limits")) {
                    constraint->setAngleLimits(glm::radians(newLower), glm::radians(newUpper));
                }

                static float motorVel = 1.0f;
                static float motorImpulse = 10.0f;
                ImGui::SliderFloat("Motor Velocity", &motorVel, -10.0f, 10.0f);
                ImGui::SliderFloat("Motor Impulse", &motorImpulse, 0.0f, 100.0f);
                if (ImGui::Button("Enable Motor")) {
                    constraint->enableMotor(motorVel, motorImpulse);
                }
                ImGui::SameLine();
                if (ImGui::Button("Disable Motor")) {
                    constraint->disableMotor();
                }
                break;
            }

            case ConstraintType::SLIDER: {
                float currentPos = constraint->getSliderPosition();
                ImGui::Text("Current Position: %.2f", currentPos);

                static float newLowerLin = 0.0f;
                static float newUpperLin = 2.0f;
                ImGui::SliderFloat("Lower Limit", &newLowerLin, -10.0f, 10.0f);
                ImGui::SliderFloat("Upper Limit", &newUpperLin, -10.0f, 10.0f);
                if (ImGui::Button("Apply Limits")) {
                    constraint->setLinearLimits(newLowerLin, newUpperLin);
                }

                static float linMotorVel = 1.0f;
                static float linMotorForce = 10.0f;
                ImGui::SliderFloat("Motor Velocity", &linMotorVel, -10.0f, 10.0f);
                ImGui::SliderFloat("Motor Force", &linMotorForce, 0.0f, 100.0f);
                if (ImGui::Button("Enable Linear Motor")) {
                    constraint->enableLinearMotor(linMotorVel, linMotorForce);
                }
                break;
            }

            case ConstraintType::SPRING: {
                static int axis = 1; // Y axis
                static float stiffness = 100.0f;
                static float damping = 10.0f;

                ImGui::SliderInt("Axis (0=X,1=Y,2=Z)", &axis, 0, 5);
                ImGui::SliderFloat("Stiffness", &stiffness, 1.0f, 1000.0f);
                ImGui::SliderFloat("Damping", &damping, 0.1f, 100.0f);

                if (ImGui::Button("Apply Spring Settings")) {
                    constraint->setSpringStiffness(axis, stiffness);
                    constraint->setSpringDamping(axis, damping);
                }
                break;
            }

            default:
                ImGui::TextDisabled("No runtime controls for this type");
                break;
            }

            ImGui::Separator();

            if (ImGui::Button("Remove This Constraint", ImVec2(-1, 0))) {
                if (context.constraintCommands.removeConstraint) {
                    context.constraintCommands.removeConstraint(constraint);
                    selectedConstraintIndex = -1;
                }
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Remove All Constraints from Object", ImVec2(-1, 0))) {
            if (context.constraintCommands.removeConstraintsForObject) {
                context.constraintCommands.removeConstraintsForObject(context.selectedObject);
                selectedConstraintIndex = -1;
            }
        }
    }

    ImGui::End();
}

void DebugUI::draw(const DebugUIContext& context)
{

    // Stats Panel

    // Displays read-only performance and physics information.
    // This panel does NOT modify engine state — it only reflects it.
    // All data comes from DebugUIContext, keeping UI decoupled from systems.
    ImGui::Begin("Stats");

    // Frame timing information
    ImGui::Text("FPS: %.1f", context.time.fps);
    ImGui::Text("Delta Time: %.4f s", context.time.deltaTime);

    ImGui::Separator();

    // Physics debug information
    ImGui::Text("Rigid Bodies: %d", context.physics.rigidBodyCount);
    ImGui::Text("Physics Enabled: %s", context.physics.physicsEnabled ? "Yes" : "No");

    ImGui::End();

    // ============================
    // Spawn Controls
    
    // Editor-style controls for requesting new objects in the scene.
    // DebugUI does NOT create objects directly — it issues a request
    // through the SceneDebugCommands interface provided by the Engine.
    ImGui::Begin("Spawn");

    // Spawn parameters (UI-owned state)
    // These are static so values persist across frames
    static int selectedShape = 0;  // 0=Cube, 1=Sphere, 2=Capsule
    static float spawnPos[3] = { 0.0f, 5.0f, 0.0f };
    static float mass = 1.0f;

    // Physics spawn
    static bool spawnWithPhysics = true;
    ImGui::Checkbox("Enable Physics", &spawnWithPhysics);

    // Shape-specific parameters
	static float cubeSize[3] = { 1.0f, 1.0f, 1.0f }; // cubes use half-extents
    static float sphereRadius = 1.0f; 
    static float capsuleRadius = 0.5f;
    static float capsuleHeight = 2.0f; 

    // Material selection
    static int selectedMaterialIndex = 0;
    static bool useCustomMaterial = false;
    static char customMaterialName[64] = "Custom";
    static float customFriction = 0.5f;
    static float customRestitution = 0.3f;

    // Texture selection
    static int selectedTextureIndex = 0;
    static std::vector<std::string> availableTextures;

    // GEt available texture 
    static bool texturesLoaded = false;
    if (!texturesLoaded && context.scene.getAvailableTextures) {
        availableTextures = context.scene.getAvailableTextures();
        texturesLoaded = true;
    }

    // Shape selection dropdown
    ImGui::SeparatorText("Shape");
    const char* shapes[] = { "Cube","Sphere","Capsule" };
    ImGui::Combo("Type", &selectedShape, shapes, IM_ARRAYSIZE(shapes));

    // Shape Parameters
    ImGui::SeparatorText("Dimensions");
    switch (selectedShape) {
    case 0://cube
        ImGui::InputFloat3("Size", cubeSize);
        break;
    case 1://sphere
        ImGui::InputFloat("Radius", &sphereRadius);
        break;
    case 2://capsule
        ImGui::InputFloat("Radius", &capsuleRadius);
        ImGui::InputFloat("Height", &capsuleHeight);
        break;

    }

    // Transforms and Physics
    ImGui::SeparatorText("Transform & Physics");

    // Position input for new object
    ImGui::InputFloat3("Position", spawnPos);
    //Mass input (0 = static object)
    ImGui::InputFloat("Mass (0 = static)", &mass);

    if (mass < 0.0f) mass = 0.0f; // Clamp to non-negative

    //Material selection
    ImGui::SeparatorText("Material");
    ImGui::Checkbox("Custom Material", &useCustomMaterial);

    if (useCustomMaterial) {
        ImGui::InputText("Name", customMaterialName, IM_ARRAYSIZE(customMaterialName));
        ImGui::SliderFloat("Friction", &customFriction, 0.0f, 2.0f);
        ImGui::SliderFloat("Restitution", &customRestitution, 0.0f, 1.0f);

        // Save custom material
        if (ImGui::Button("Save to Presets")) {
            if (context.scene.registerMaterial) {
                context.scene.registerMaterial(
                    std::string(customMaterialName),
                    customFriction,
                    customRestitution
                );
            }
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Saves this material to the registry for future use");
        }
    }
    else {
        // preset material selection
        if (!context.physics.availableMaterials.empty()){
            // Build array of C-style strings for ImGui Combo
            // ImGui requires const char* array 
            std::vector<const char*> materialNames;
            for (const auto& mat : context.physics.availableMaterials) {
                materialNames.push_back(mat.c_str());
            }
            ImGui::Combo("Preset Material", &selectedMaterialIndex, materialNames.data(), static_cast<int>(materialNames.size()));
        }
    }
    // Texture selection
    if (!availableTextures.empty()) {
        // add "None" option at start
        std::vector<const char*> textureNames;
        textureNames.push_back("");
        for (const auto& tex : availableTextures) {
            textureNames.push_back(tex.c_str());
        }
        ImGui::Combo("Texture", &selectedTextureIndex, textureNames.data(), static_cast<int>(textureNames.size()));
    }
    else {
        ImGui::TextDisabled("No textures found");
    }
    ImGui::Separator();

    // Spawn Button
    if (ImGui::Button("Spawn Object", ImVec2(-1, 0))){
        if (context.scene.spawnObject) {
            //Determine shape
			ShapeType shapeType;
            glm::vec3 size;

            switch (selectedShape) {
                case 0://cube
                    shapeType = ShapeType::CUBE;
                    size = glm::vec3(cubeSize[0], cubeSize[1], cubeSize[2]);
					break;
                case 1://sphere
                    shapeType = ShapeType::SPHERE;
					size = glm::vec3(sphereRadius);
                    break;
                case 2://capsule
                    shapeType = ShapeType::CAPSULE;
                    size = glm::vec3(capsuleRadius, capsuleHeight, capsuleRadius);
                    break;
                default:
					shapeType = ShapeType::CUBE;
					size = glm::vec3(1.0f);
            }

			// Determine material
			std::string materialName;
            if (useCustomMaterial) {
				materialName = std::string(customMaterialName);
                //register if not already registered
                if (context.scene.registerMaterial) {
                    context.scene.registerMaterial(
                        materialName,
                        customFriction,
                        customRestitution
                    );
                }

            }else {
                if (selectedMaterialIndex >= 0 &&
                    selectedMaterialIndex < static_cast<int>(context.physics.availableMaterials.size())) {
                    materialName = context.physics.availableMaterials[selectedMaterialIndex];
                }
                else {
                    materialName = "Default";
                }
            }
            // Determine texture path
            std::string texturePath = "";
            if (selectedTextureIndex > 0 && selectedTextureIndex <= static_cast<int>(availableTextures.size())) {
                texturePath = availableTextures[selectedTextureIndex - 1];
            }

            // Spawn the object
            if (spawnWithPhysics)
            {
                context.scene.spawnObject(
                    shapeType,
                    glm::vec3(spawnPos[0], spawnPos[1], spawnPos[2]),
                    size,
                    mass,
                    materialName,
                    texturePath
                );
            }
            else
            {
                context.scene.spawnRenderObject(
                    shapeType,
                    glm::vec3(spawnPos[0], spawnPos[1], spawnPos[2]),
                    size,
                    texturePath
                );
            }

        }

    }

    ImGui::End();

    // ============================
    // Inspector
    //
    // Shows details for the currently selected object.
    // This is the first step towards a real editor workflow:
    // click in scene -> inspect -> edit -> see changes live.
    ImGui::Begin("Inspector");

    // Nothing selected -> show hint and exit early.
    // Prevents null pointer access and keeps UI clear.
    if (!context.selectedObject)
    {
        ImGui::Text("No object selected.");
        ImGui::Text("Click an object in the scene to inspect it.");
    }

    else {
        // Pull current transform from the selected object.
        // We copy into local values because ImGui works with plain floats.
        glm::vec3 pos = context.selectedObject->getPosition();
        glm::vec3 scale = context.selectedObject->getScale();
        glm::quat rot = context.selectedObject->getRotation();



        ImGui::Text("Selected Object");
        ImGui::Separator();

        // Physics status display
        if (context.selectedObject->isRenderOnly())
        {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Physics: Disabled (Render Only)");
        }
        else
        {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Physics: Enabled");
        }

        // ----------------------------
        // Position
        // ----------------------------
        // Convert glm::vec3 -> float[3] for ImGui controls.
        float posArr[3] = { pos.x, pos.y, pos.z };

        // If user edits the values, write them back into the GameObject.
        if (ImGui::DragFloat3("Position", posArr, 0.05f))
        {
            context.selectedObject->setPosition(glm::vec3(posArr[0], posArr[1], posArr[2]));
        }

        // ----------------------------
        // Rotation
        // ----------------------------
        // Convert quaternion -> Euler (degrees) for UI editing.
        glm::vec3 eulerRad = QuatToEulerRad(rot);
        glm::vec3 eulerDeg = glm::degrees(eulerRad);

        float rotArr[3] = { eulerDeg.x, eulerDeg.y, eulerDeg.z };
        if (ImGui::DragFloat3("Rotation (deg)", rotArr, 0.5f))
        {
            glm::vec3 newEulerRad = glm::radians(glm::vec3(rotArr[0], rotArr[1], rotArr[2]));
            context.selectedObject->setRotation(EulerRadToQuat(newEulerRad));
        }

        // ----------------------------
        // Scale
        // ----------------------------
        // Same approach as position, but clamp scale to avoid negatives/zero.
        float scaleArr[3] = { scale.x, scale.y, scale.z };

        if (ImGui::DragFloat3("Scale", scaleArr, 0.05f, 0.01f, 1000.0f))
        {
            if (context.scene.setObjectScale) {
                context.scene.setObjectScale(
                    context.selectedObject,
                    glm::vec3(scaleArr[0], scaleArr[1], scaleArr[2])
                );
            }
        }
    }
    ImGui::End();


    // Lighting Controls
    ImGui::Begin("Lighting");

    if (context.lighting.getLight) {
        DirectionalLight& light = context.lighting.getLight();

        // Direction control
        glm::vec3 dir = light.getDirection();
        float dirArr[3] = { dir.x, dir.y, dir.z };

        if (ImGui::DragFloat3("Direction", dirArr, 0.01f, -1.0f, 1.0f)) {
            light.setDirection(glm::vec3(dirArr[0], dirArr[1], dirArr[2]));
        }

        // Color control
        glm::vec3 col = light.getColor();
        float colArr[3] = { col.r, col.g, col.b };

        if (ImGui::ColorEdit3("Color", colArr)) {
            light.setColor(glm::vec3(colArr[0], colArr[1], colArr[2]));
        }

        // Intensity control
        float intensity = light.getIntensity();
        if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 5.0f)) {
            light.setIntensity(intensity);
        }

        ImGui::Separator();
        ImGui::Text("Presets:");

        if (ImGui::Button("Noon Sun")) {
            light.setDirection(glm::vec3(0.0f, -1.0f, 0.0f));
            light.setColor(glm::vec3(1.0f, 1.0f, 0.9f));
            light.setIntensity(1.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Sunset")) {
            light.setDirection(glm::vec3(1.0f, -0.3f, 0.0f));
            light.setColor(glm::vec3(1.0f, 0.6f, 0.3f));
            light.setIntensity(0.8f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Night")) {
            light.setDirection(glm::vec3(0.2f, -1.0f, 0.3f));
            light.setColor(glm::vec3(0.3f, 0.3f, 0.5f));
            light.setIntensity(0.3f);
        }
    }

    ImGui::End();

    // Model Importer
    ImGui::Begin("Model Importer");

    static int selectedModelIndex = 0;
    static std::vector<std::string> availableModels;

    // Load available models
    static bool modelsLoaded = false;
    if (!modelsLoaded && context.scene.getAvailableModels) {
        availableModels = context.scene.getAvailableModels();
        modelsLoaded = true;
    }

    static float modelPos[3] = { 0.0f, 2.0f, 0.0f };
    static float modelScale[3] = { 1.0f, 1.0f, 1.0f };

    ImGui::Text("Load .obj Model");
    ImGui::Separator();

    // Dropdown for model selection
    if (!availableModels.empty()) {
        std::vector<const char*> modelNames;
        for (const auto& model : availableModels) {
            modelNames.push_back(model.c_str());
        }

        ImGui::Combo("Model File", &selectedModelIndex, modelNames.data(), static_cast<int>(modelNames.size()));
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No .obj files found in models/");
    }

    ImGui::InputFloat3("Position", modelPos);
    ImGui::InputFloat3("Scale", modelScale);

    ImGui::Separator();

    if (ImGui::Button("Load & Spawn Model", ImVec2(-1, 0))) {
        if (context.scene.loadAndSpawnModel && selectedModelIndex >= 0 && selectedModelIndex < availableModels.size()) {
            GameObject* obj = context.scene.loadAndSpawnModel(
                availableModels[selectedModelIndex],
                glm::vec3(modelPos[0], modelPos[1], modelPos[2]),
                glm::vec3(modelScale[0], modelScale[1], modelScale[2])
            );

            if (obj) {
                std::cout << "Model loaded successfully!" << std::endl;
            }
            else {
                std::cout << "Failed to load model." << std::endl;
            }
        }
    }

    ImGui::Separator();
    ImGui::TextWrapped("Place .obj files in the models/ folder. Restart to refresh list.");

    DrawConstraintCreator(context);
    DrawConstraintPresets(context);
    DrawConstraintList(context);
    DrawConstraintInspector(context);

    ImGui::End();
}
