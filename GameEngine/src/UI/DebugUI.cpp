#include "../include/Debug/DebugUI.h"
#include "../include/Rendering/DirectionalLight.h"
#include "../include/Physics/Constraint.h"
#include "../include/Physics/ConstraintRegistry.h"
#include "../include/Physics/ConstraintTemplate.h"
#include <glm/gtc/constants.hpp>
#include "../External/imgui/core/imgui.h"
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
    static int createConstraintTypeIndex = 0;
    static char constraintName[64] = "";
    // template selection 
    static char templateName[64] = "";
    static char templateDesc[256] = "";

    // tabs for create vs templates
    if (ImGui::BeginTabBar("CreatorTabs")) {

        // create new constraint tab
        if (ImGui::BeginTabItem("Create")) {
            ImGui::SeparatorText("Object Selection");

            // Display selected objects
            if (objectA) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Object A: Selected");
                if (ImGui::Button("Clear Object A##CreateClearA")) {
                    objectA = nullptr;
                }
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Object A: None");
            }

            if (objectB) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Object B: Selected");
                if (ImGui::Button("Clear Object B##CreateClearB")) {
                    objectB = nullptr;
                }
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Object B: None (Optional)");
            }

            ImGui::Separator();

            // Quick select from inspector
            if (context.selectedObject && context.selectedObject->hasPhysics()) {
                if (ImGui::Button("Set Selected as Object A##CreateA")) {
                    objectA = context.selectedObject;
                }
                ImGui::SameLine();
                if (ImGui::Button("Set Selected as Object B##CreateB")) {
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
            ImGui::Combo("Type##CreateType", &createConstraintTypeIndex, types, IM_ARRAYSIZE(types));

            // Optional name
            ImGui::InputText("Name (Optional)##CreateName", constraintName, IM_ARRAYSIZE(constraintName));

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

            // Generic 6DOF parameters
            static bool useLinearX = false;
            static float linearXLower = -1.0f;
            static float linearXUpper = 1.0f;

            static bool useLinearY = false;
            static float linearYLower = -1.0f;
            static float linearYUpper = 1.0f;

            static bool useLinearZ = false;
            static float linearZLower = -1.0f;
            static float linearZUpper = 1.0f;

            static bool useAngularX = false;
            static float angularXLower = -45.0f;
            static float angularXUpper = 45.0f;

            static bool useAngularY = false;
            static float angularYLower = -45.0f;
            static float angularYUpper = 45.0f;

            static bool useAngularZ = false;
            static float angularZLower = -45.0f;
            static float angularZUpper = 45.0f;

            static bool breakable = false;
            static float breakForce = 1000.0f;
            static float breakTorque = 1000.0f;

            switch (createConstraintTypeIndex) {
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

                ImGui::Text("Linear Axes:");
                ImGui::Checkbox("X##SpringLinX", &springAxisEnabled[0]); ImGui::SameLine();
                ImGui::Checkbox("Y##SpringLinY", &springAxisEnabled[1]); ImGui::SameLine();
                ImGui::Checkbox("Z##SpringLinZ", &springAxisEnabled[2]);

                ImGui::Text("Angular Axes:");
                ImGui::Checkbox("Rot X##SpringAngX", &springAxisEnabled[3]); ImGui::SameLine();
                ImGui::Checkbox("Rot Y##SpringAngY", &springAxisEnabled[4]); ImGui::SameLine();
                ImGui::Checkbox("Rot Z##SpringAngZ", &springAxisEnabled[5]);
                break;

            case 4: // Generic 6DOF
                ImGui::TextWrapped("Advanced constraint with full control over all 6 degrees of freedom.");

                ImGui::Separator();
                ImGui::Text("Linear Limits (Translation)");

                ImGui::Checkbox("Limit X Axis", &useLinearX);
                if (useLinearX) {
                    ImGui::SliderFloat("X Lower", &linearXLower, -10.0f, 0.0f);
                    ImGui::SliderFloat("X Upper", &linearXUpper, 0.0f, 10.0f);
                }

                ImGui::Checkbox("Limit Y Axis", &useLinearY);
                if (useLinearY) {
                    ImGui::SliderFloat("Y Lower", &linearYLower, -10.0f, 0.0f);
                    ImGui::SliderFloat("Y Upper", &linearYUpper, 0.0f, 10.0f);
                }

                ImGui::Checkbox("Limit Z Axis", &useLinearZ);
                if (useLinearZ) {
                    ImGui::SliderFloat("Z Lower", &linearZLower, -10.0f, 0.0f);
                    ImGui::SliderFloat("Z Upper", &linearZUpper, 0.0f, 10.0f);
                }

                ImGui::Separator();
                ImGui::Text("Angular Limits (Rotation in degrees)");

                ImGui::Checkbox("Limit X Rotation", &useAngularX);
                if (useAngularX) {
                    ImGui::SliderFloat("X Rot Lower", &angularXLower, -180.0f, 0.0f);
                    ImGui::SliderFloat("X Rot Upper", &angularXUpper, 0.0f, 180.0f);
                }

                ImGui::Checkbox("Limit Y Rotation", &useAngularY);
                if (useAngularY) {
                    ImGui::SliderFloat("Y Rot Lower", &angularYLower, -180.0f, 0.0f);
                    ImGui::SliderFloat("Y Rot Upper", &angularYUpper, 0.0f, 180.0f);
                }

                ImGui::Checkbox("Limit Z Rotation", &useAngularZ);
                if (useAngularZ) {
                    ImGui::SliderFloat("Z Rot Lower", &angularZLower, -180.0f, 0.0f);
                    ImGui::SliderFloat("Z Rot Upper", &angularZUpper, 0.0f, 180.0f);
                }
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

                switch (createConstraintTypeIndex) {
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

                        // Set linear limits
                        params.useLinearLimits[0] = useLinearX;
                        params.lowerLinearLimit[0] = linearXLower;
                        params.upperLinearLimit[0] = linearXUpper;

                        params.useLinearLimits[1] = useLinearY;
                        params.lowerLinearLimit[1] = linearYLower;
                        params.upperLinearLimit[1] = linearYUpper;

                        params.useLinearLimits[2] = useLinearZ;
                        params.lowerLinearLimit[2] = linearZLower;
                        params.upperLinearLimit[2] = linearZUpper;

                        // Set angular limits (convert degrees to radians)
                        params.useAngularLimits[0] = useAngularX;
                        params.lowerAngularLimit[0] = glm::radians(angularXLower);
                        params.upperAngularLimit[0] = glm::radians(angularXUpper);

                        params.useAngularLimits[1] = useAngularY;
                        params.lowerAngularLimit[1] = glm::radians(angularYLower);
                        params.upperAngularLimit[1] = glm::radians(angularYUpper);

                        params.useAngularLimits[2] = useAngularZ;
                        params.lowerAngularLimit[2] = glm::radians(angularZLower);
                        params.upperAngularLimit[2] = glm::radians(angularZUpper);

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

            // Save as template button
            ImGui::Separator();
            ImGui::SeparatorText("Save as Template");
            ImGui::InputText("Template Name##SaveTemplateName", templateName, IM_ARRAYSIZE(templateName));
            ImGui::InputText("Description##SaveTemplateDesc", templateDesc, IM_ARRAYSIZE(templateDesc));

            bool canSaveTemplate = templateName[0] != '\0';
            ImGui::BeginDisabled(!canSaveTemplate);

            if (ImGui::Button("Save Current Settings as Template", ImVec2(-1, 0))) {
                ConstraintTemplate templ;
                templ.name = templateName;
                templ.description = templateDesc;
                templ.type = static_cast<ConstraintType>(createConstraintTypeIndex);
                templ.breakable = breakable;
                templ.breakForce = breakForce;
                templ.breakTorque = breakTorque;

                // Save type-specific params
                switch (createConstraintTypeIndex) {
                case 1: // Hinge
                    templ.hingeParams.pivotA = glm::vec3(hingePivot[0], hingePivot[1], hingePivot[2]);
                    templ.hingeParams.axisA = glm::normalize(glm::vec3(hingeAxis[0], hingeAxis[1], hingeAxis[2]));
                    templ.hingeParams.useLimits = useHingeLimits;
                    templ.hingeParams.lowerLimit = glm::radians(hingeLowerLimit);
                    templ.hingeParams.upperLimit = glm::radians(hingeUpperLimit);
                    templ.hingeParams.useMotor = useHingeMotor;
                    templ.hingeParams.motorTargetVelocity = hingeMotorVelocity;
                    templ.hingeParams.motorMaxImpulse = hingeMotorForce;
                    break;
                case 2: // Slider
                    templ.sliderParams.useLimits = true;
                    templ.sliderParams.lowerLimit = 0.0f;
                    templ.sliderParams.upperLimit = sliderDistance;
                    templ.sliderParams.useMotor = useSliderMotor;
                    templ.sliderParams.motorTargetVelocity = sliderMotorVelocity;
                    templ.sliderParams.motorMaxForce = sliderMotorForce;
                    break;
                case 3: // Spring
                    for (int i = 0; i < 6; ++i) {
                        templ.springParams.enableSpring[i] = springAxisEnabled[i];
                        templ.springParams.stiffness[i] = springStiffness;
                        templ.springParams.damping[i] = springDamping;
                    }
                    break;
                case 4: // Generic 6DOF
                    templ.dofParams.useLinearLimits[0] = useLinearX;
                    templ.dofParams.lowerLinearLimit[0] = linearXLower;
                    templ.dofParams.upperLinearLimit[0] = linearXUpper;
                    templ.dofParams.useLinearLimits[1] = useLinearY;
                    templ.dofParams.lowerLinearLimit[1] = linearYLower;
                    templ.dofParams.upperLinearLimit[1] = linearYUpper;
                    templ.dofParams.useLinearLimits[2] = useLinearZ;
                    templ.dofParams.lowerLinearLimit[2] = linearZLower;
                    templ.dofParams.upperLinearLimit[2] = linearZUpper;
                    templ.dofParams.useAngularLimits[0] = useAngularX;
                    templ.dofParams.lowerAngularLimit[0] = glm::radians(angularXLower);
                    templ.dofParams.upperAngularLimit[0] = glm::radians(angularXUpper);
                    templ.dofParams.useAngularLimits[1] = useAngularY;
                    templ.dofParams.lowerAngularLimit[1] = glm::radians(angularYLower);
                    templ.dofParams.upperAngularLimit[1] = glm::radians(angularYUpper);
                    templ.dofParams.useAngularLimits[2] = useAngularZ;
                    templ.dofParams.lowerAngularLimit[2] = glm::radians(angularZLower);
                    templ.dofParams.upperAngularLimit[2] = glm::radians(angularZUpper);
                    break;
                }

                ConstraintTemplateRegistry::getInstance().addTemplate(templ);
                ConstraintTemplateRegistry::getInstance().save();

                std::cout << "Template '" << templateName << "' saved!" << std::endl;

                // Clear template name field
                templateName[0] = '\0';
                templateDesc[0] = '\0';
            }

            ImGui::EndDisabled();

            ImGui::EndTabItem();
        }

        // templates tab
        if (ImGui::BeginTabItem("Templates")) {
            ImGui::TextWrapped("Load saved constraint templates and apply them to selected objects.");
            ImGui::Separator();

            auto& registry = ConstraintTemplateRegistry::getInstance();
            auto templates = registry.getAllTemplates();

            if (templates.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No templates saved yet.");
                ImGui::Text("Create a constraint and save it as a template!");
            }
            else {
                ImGui::Text("Available Templates: %d", (int)templates.size());
                ImGui::Separator();

                static int selectedTemplate = 0;

                // Template list
                if (ImGui::BeginListBox("##Templates", ImVec2(-1, 200))) {
                    for (int i = 0; i < templates.size(); ++i) {
                        const auto& templ = templates[i];
                        ImGui::PushID(i);
                        bool isSelected = (selectedTemplate == i);
                        std::string label = templ.name + " (" + ConstraintTypeToString(templ.type) + ")";

                        if (ImGui::Selectable(label.c_str(), isSelected)) {
                            selectedTemplate = i;
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndListBox();
                }

                // Template details
                if (selectedTemplate >= 0 && selectedTemplate < templates.size()) {
                    const auto& templ = templates[selectedTemplate];

                    ImGui::SeparatorText("Template Details");
                    ImGui::Text("Name: %s", templ.name.c_str());
                    ImGui::Text("Type: %s", ConstraintTypeToString(templ.type));
                    if (!templ.description.empty()) {
                        ImGui::TextWrapped("Description: %s", templ.description.c_str());
                    }

                    ImGui::Separator();

                    // Object selection for applying template
                    ImGui::SeparatorText("Apply Template");

                    if (objectA) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Object A: ID %llu", objectA->getID());
                    }
                    else {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Object A: None");
                    }

                    if (objectB) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Object B: ID %llu", objectB->getID());
                    }
                    else {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Object B: None (Optional)");
                    }

                    if (context.selectedObject && context.selectedObject->hasPhysics()) {
                        if (ImGui::Button("Set Selected as Object A##Template")) {
                            objectA = context.selectedObject;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Set Selected as Object B##Template")) {
                            objectB = context.selectedObject;
                        }
                    }

                    ImGui::Separator();

                    bool canApply = objectA != nullptr && objectA->hasPhysics();
                    ImGui::BeginDisabled(!canApply);

                    if (ImGui::Button("Apply Template to Objects", ImVec2(-1, 0))) {
                        auto constraint = registry.applyTemplate(templ.name, objectA, objectB);

                        if (constraint) {
                            // Template creates the constraint after its added to the registry
                            // The ConstraintRegistry::getInstance().addConstraint() 
                            ConstraintRegistry::getInstance().addConstraint(std::move(constraint));
                            std::cout << "Applied template '" << templ.name << "'" << std::endl;
                        }
                    }

                    ImGui::EndDisabled();

                    ImGui::Separator();

                    // Delete template button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                    if (ImGui::Button("Delete Template", ImVec2(-1, 0))) {
                        registry.removeTemplate(templ.name);
                        registry.save();
                        selectedTemplate = 0;
                        std::cout << "Deleted template '" << templ.name << "'" << std::endl;
                    }
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }
    ImGui::End();
}
static void DrawUnifiedInspector(const DebugUIContext& context) {
    ImGui::Begin("Inspector");

    if (!context.selectedObject) {
        ImGui::Text("No object selected.");
        ImGui::Text("Click an object in the scene to inspect it.");
        ImGui::End();
        return;
    }

    // object properties

    glm::vec3 pos = context.selectedObject->getPosition();
    glm::vec3 scale = context.selectedObject->getScale();
    glm::quat rot = context.selectedObject->getRotation();

    ImGui::Text("Object ID: %llu", context.selectedObject->getID());
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {

        // Physics status
        if (context.selectedObject->isRenderOnly()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Physics: Disabled (Render Only)");
        }
        else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Physics: Enabled");
        }

        // Position
        float posArr[3] = { pos.x, pos.y, pos.z };
        if (ImGui::DragFloat3("Position", posArr, 0.05f)) {
            context.selectedObject->setPosition(glm::vec3(posArr[0], posArr[1], posArr[2]));
        }

        // Rotation
        glm::vec3 eulerRad = QuatToEulerRad(rot);
        glm::vec3 eulerDeg = glm::degrees(eulerRad);
        float rotArr[3] = { eulerDeg.x, eulerDeg.y, eulerDeg.z };
        if (ImGui::DragFloat3("Rotation (deg)", rotArr, 0.5f)) {
            glm::vec3 newEulerRad = glm::radians(glm::vec3(rotArr[0], rotArr[1], rotArr[2]));
            context.selectedObject->setRotation(EulerRadToQuat(newEulerRad));
        }

        // Scale
        float scaleArr[3] = { scale.x, scale.y, scale.z };
        if (ImGui::DragFloat3("Scale", scaleArr, 0.05f, 0.01f, 1000.0f)) {
            if (context.scene.setObjectScale) {
                context.scene.setObjectScale(
                    context.selectedObject,
                    glm::vec3(scaleArr[0], scaleArr[1], scaleArr[2])
                );
            }
        }
    }

    // constraint section
    if (ImGui::CollapsingHeader("Constraints")) {
        auto constraints = ConstraintRegistry::getInstance().findConstraintsByObject(context.selectedObject);

        if (constraints.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No constraints attached");
        }
        else {
            ImGui::Text("Attached Constraints: %d", (int)constraints.size());
            ImGui::Separator();

            for (int i = 0; i < constraints.size(); ++i) {
                Constraint* c = constraints[i];

                ImGui::PushID(i);

                std::string label = c->getName().empty() ?
                    ("Constraint " + std::to_string(i)) : c->getName();

                if (ImGui::TreeNode(label.c_str())) {
                    ImGui::Text("Type: %s", ConstraintTypeToString(c->getType()));

                    // Show connected objects
                    GameObject* bodyA = c->getBodyA();
                    GameObject* bodyB = c->getBodyB();

                    if (bodyA) {
                        ImGui::Text("Connected to A: ID %llu", bodyA->getID());
                    }
                    if (bodyB) {
                        ImGui::Text("Connected to B: ID %llu", bodyB->getID());
                    }
                    else {
                        ImGui::Text("Connected to: World");
                    }

                    // Enable/disable
                    bool enabled = !c->isBroken();
                    if (ImGui::Checkbox("Enabled", &enabled)) {
                        c->setEnabled(enabled);
                    }

                    // Type-specific controls
                    if (c->getType() == ConstraintType::HINGE) {
                        float angle = glm::degrees(c->getHingeAngle());
                        ImGui::Text("Current Angle: %.1f°", angle);
                    }
                    else if (c->getType() == ConstraintType::SLIDER) {
                        float pos = c->getSliderPosition();
                        ImGui::Text("Current Position: %.2f", pos);
                    }

                    // Breakable info
                    if (c->isBreakable()) {
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Breakable");
                        ImGui::Text("Break Force: %.0f", c->getBreakForce());
                    }

                    // Remove button
                    ImGui::Separator();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                    if (ImGui::Button("Remove Constraint", ImVec2(-1, 0))) {
                        if (context.constraintCommands.removeConstraint) {
                            context.constraintCommands.removeConstraint(c);
                        }
                    }
                    ImGui::PopStyleColor();

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }
        }
    }
    

    ImGui::Separator();

    // DELETE OBJECT 

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));

    if (ImGui::Button("Delete Object", ImVec2(-1, 0))) {
        if (context.scene.destroyObject) {
            context.scene.destroyObject(context.selectedObject);
        }
    }

    ImGui::PopStyleColor(3);

    ImGui::End();
}

void DebugUI::draw(const DebugUIContext& context)
{
    // ============================
    // CREATE DOCKSPACE FIRST
    // ============================
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags dockspace_flags = ImGuiWindowFlags_NoDocking;
    dockspace_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    dockspace_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    dockspace_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, dockspace_flags);
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();


   
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

    DrawUnifiedInspector(context);
    DrawConstraintCreator(context);

    ImGui::End();
}
