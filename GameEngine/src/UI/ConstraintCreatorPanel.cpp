#include "../include/UI/ConstraintCreatorPanel.h"
#include "../include/Physics/Constraint.h"
#include "../include/Physics/ConstraintRegistry.h"
#include "../include/Physics/ConstraintTemplate.h"
#include "../External/imgui/core/imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>

//  Local helper
static const char* ConstraintTypeToStr(ConstraintType type)
{
    switch (type)
    {
    case ConstraintType::FIXED:        return "Fixed";
    case ConstraintType::HINGE:        return "Hinge";
    case ConstraintType::SLIDER:       return "Slider";
    case ConstraintType::SPRING:       return "Spring";
    case ConstraintType::GENERIC_6DOF: return "Generic 6DOF";
    default:                           return "Unknown";
    }
}

//  DrawConstraintCreatorPanel
void DrawConstraintCreatorPanel(DebugUIContext& context)
{
    ImGui::Begin("Constraint Creator");

    // Shared object selection state (persists across tabs)
    static GameObject* objectA = nullptr;
    static GameObject* objectB = nullptr;

    //  Tab bar
    if (!ImGui::BeginTabBar("CreatorTabs"))
    {
        ImGui::End();
        return;
    }

    //  CREATE TAB
    if (ImGui::BeginTabItem("Create"))
    {
        static int  createConstraintTypeIndex = 0;
        static char constraintName[64] = "";

        // Hinge
        static float hingeAxis[3] = { 0.0f, 1.0f, 0.0f };
        static float hingePivot[3] = { 0.0f, 0.0f, 0.0f };
        static bool  useHingeLimits = false;
        static float hingeLowerLimit = 0.0f;
        static float hingeUpperLimit = 90.0f;
        static bool  useHingeMotor = false;
        static float hingeMotorVelocity = 1.0f;
        static float hingeMotorForce = 10.0f;

        // Slider
        static float sliderDistance = 2.0f;
        static bool  useSliderMotor = false;
        static float sliderMotorVelocity = 1.0f;
        static float sliderMotorForce = 10.0f;

        // Spring
        static float springStiffness = 100.0f;
        static float springDamping = 10.0f;
        static bool  springAxisEnabled[6] = { false, true, false, false, false, false };

        // Generic 6DOF
        static bool  useLinearX = false; static float linearXLower = -1.0f; static float linearXUpper = 1.0f;
        static bool  useLinearY = false; static float linearYLower = -1.0f; static float linearYUpper = 1.0f;
        static bool  useLinearZ = false; static float linearZLower = -1.0f; static float linearZUpper = 1.0f;
        static bool  useAngularX = false; static float angularXLower = -45.0f; static float angularXUpper = 45.0f;
        static bool  useAngularY = false; static float angularYLower = -45.0f; static float angularYUpper = 45.0f;
        static bool  useAngularZ = false; static float angularZLower = -45.0f; static float angularZUpper = 45.0f;

        // Breaking
        static bool  breakable = false;
        static float breakForce = 1000.0f;
        static float breakTorque = 1000.0f;

        //  Object selection 
        ImGui::SeparatorText("Object Selection");

        if (objectA)
        {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Object A: Selected");
            if (ImGui::Button("Clear Object A##CreateClearA")) objectA = nullptr;
        }
        else ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Object A: None");

        if (objectB)
        {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Object B: Selected");
            if (ImGui::Button("Clear Object B##CreateClearB")) objectB = nullptr;
        }
        else ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Object B: None (Optional)");

        ImGui::Separator();
        if (context.selectedObject && context.selectedObject->hasPhysics())
        {
            if (ImGui::Button("Set Selected as Object A##CreateA")) objectA = context.selectedObject;
            ImGui::SameLine();
            if (ImGui::Button("Set Selected as Object B##CreateB")) objectB = context.selectedObject;
        }
        else ImGui::TextDisabled("Select a physics object in the scene");

        // Type selection
        ImGui::SeparatorText("Constraint Type");
        const char* types[] = { "Fixed Joint", "Hinge (Door/Wheel)",
                                 "Slider (Drawer)", "Spring (Suspension)", "Generic 6DOF" };
        ImGui::Combo("Type##CreateType", &createConstraintTypeIndex,
            types, IM_ARRAYSIZE(types));
        ImGui::InputText("Name (Optional)##CreateName",
            constraintName, IM_ARRAYSIZE(constraintName));

        // Type-specific parameters
        ImGui::SeparatorText("Parameters");
        switch (createConstraintTypeIndex)
        {
        case 0: // Fixed
            ImGui::TextWrapped("Fixed joints lock two objects together rigidly.");
            break;

        case 1: // Hinge
            ImGui::InputFloat3("Hinge Axis", hingeAxis);
            ImGui::InputFloat3("Pivot Point", hingePivot);
            ImGui::Checkbox("Use Limits", &useHingeLimits);
            if (useHingeLimits)
            {
                ImGui::SliderFloat("Lower Limit (deg)", &hingeLowerLimit, -180.0f, 180.0f);
                ImGui::SliderFloat("Upper Limit (deg)", &hingeUpperLimit, -180.0f, 180.0f);
            }
            ImGui::Checkbox("Use Motor", &useHingeMotor);
            if (useHingeMotor)
            {
                ImGui::SliderFloat("Motor Velocity", &hingeMotorVelocity, -10.0f, 10.0f);
                ImGui::SliderFloat("Motor Force", &hingeMotorForce, 0.0f, 100.0f);
            }
            break;

        case 2: // Slider
            ImGui::SliderFloat("Slide Distance", &sliderDistance, 0.1f, 10.0f);
            ImGui::Checkbox("Use Motor", &useSliderMotor);
            if (useSliderMotor)
            {
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
            ImGui::TextWrapped("Full control over all 6 degrees of freedom.");
            ImGui::Separator();
            ImGui::Text("Linear Limits (Translation)");
            ImGui::Checkbox("Limit X Axis", &useLinearX);
            if (useLinearX) { ImGui::SliderFloat("X Lower", &linearXLower, -10.0f, 0.0f); ImGui::SliderFloat("X Upper", &linearXUpper, 0.0f, 10.0f); }
            ImGui::Checkbox("Limit Y Axis", &useLinearY);
            if (useLinearY) { ImGui::SliderFloat("Y Lower", &linearYLower, -10.0f, 0.0f); ImGui::SliderFloat("Y Upper", &linearYUpper, 0.0f, 10.0f); }
            ImGui::Checkbox("Limit Z Axis", &useLinearZ);
            if (useLinearZ) { ImGui::SliderFloat("Z Lower", &linearZLower, -10.0f, 0.0f); ImGui::SliderFloat("Z Upper", &linearZUpper, 0.0f, 10.0f); }
            ImGui::Separator();
            ImGui::Text("Angular Limits (degrees)");
            ImGui::Checkbox("Limit X Rotation", &useAngularX);
            if (useAngularX) { ImGui::SliderFloat("X Rot Lower", &angularXLower, -180.0f, 0.0f); ImGui::SliderFloat("X Rot Upper", &angularXUpper, 0.0f, 180.0f); }
            ImGui::Checkbox("Limit Y Rotation", &useAngularY);
            if (useAngularY) { ImGui::SliderFloat("Y Rot Lower", &angularYLower, -180.0f, 0.0f); ImGui::SliderFloat("Y Rot Upper", &angularYUpper, 0.0f, 180.0f); }
            ImGui::Checkbox("Limit Z Rotation", &useAngularZ);
            if (useAngularZ) { ImGui::SliderFloat("Z Rot Lower", &angularZLower, -180.0f, 0.0f); ImGui::SliderFloat("Z Rot Upper", &angularZUpper, 0.0f, 180.0f); }
            break;
        }

        //  Breaking 
        ImGui::SeparatorText("Breaking");
        ImGui::Checkbox("Breakable", &breakable);
        if (breakable)
        {
            ImGui::SliderFloat("Break Force", &breakForce, 10.0f, 10000.0f);
            ImGui::SliderFloat("Break Torque", &breakTorque, 10.0f, 10000.0f);
        }

        ImGui::Separator();

        // Create button 
        bool canCreate = objectA && objectA->hasPhysics();
        if (!canCreate)
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Need at least Object A with physics!");

        ImGui::BeginDisabled(!canCreate);
        if (ImGui::Button("Create Constraint", ImVec2(-1, 0)))
        {
            Constraint* newConstraint = nullptr;

            switch (createConstraintTypeIndex)
            {
            case 0:
                if (context.constraintCommands.createFixed)
                    newConstraint = context.constraintCommands.createFixed(objectA, objectB);
                break;

            case 1:
                if (context.constraintCommands.createHingeAdvanced)
                {
                    HingeParams p;
                    p.pivotA = glm::vec3(hingePivot[0], hingePivot[1], hingePivot[2]);
                    p.axisA = glm::normalize(glm::vec3(hingeAxis[0], hingeAxis[1], hingeAxis[2]));
                    if (objectB) { p.pivotB = p.pivotA; p.axisB = p.axisA; }
                    p.useLimits = useHingeLimits;
                    p.lowerLimit = glm::radians(hingeLowerLimit);
                    p.upperLimit = glm::radians(hingeUpperLimit);
                    p.useMotor = useHingeMotor;
                    p.motorTargetVelocity = hingeMotorVelocity;
                    p.motorMaxImpulse = hingeMotorForce;
                    newConstraint = context.constraintCommands.createHingeAdvanced(objectA, objectB, p);
                }
                break;

            case 2:
                if (context.constraintCommands.createSlider)
                {
                    SliderParams p;
                    p.useLimits = true;
                    p.lowerLimit = 0.0f;
                    p.upperLimit = sliderDistance;
                    p.useMotor = useSliderMotor;
                    p.motorTargetVelocity = sliderMotorVelocity;
                    p.motorMaxForce = sliderMotorForce;
                    newConstraint = context.constraintCommands.createSlider(objectA, objectB, p);
                }
                break;

            case 3:
                if (context.constraintCommands.createSpringAdvanced)
                {
                    SpringParams p;
                    for (int i = 0; i < 6; i++)
                    {
                        p.enableSpring[i] = springAxisEnabled[i];
                        p.stiffness[i] = springStiffness;
                        p.damping[i] = springDamping;
                    }
                    newConstraint = context.constraintCommands.createSpringAdvanced(objectA, objectB, p);
                }
                break;

            case 4:
                if (context.constraintCommands.createGeneric6Dof)
                {
                    Generic6DofParams p;
                    p.useLinearLimits[0] = useLinearX;  p.lowerLinearLimit[0] = linearXLower;  p.upperLinearLimit[0] = linearXUpper;
                    p.useLinearLimits[1] = useLinearY;  p.lowerLinearLimit[1] = linearYLower;  p.upperLinearLimit[1] = linearYUpper;
                    p.useLinearLimits[2] = useLinearZ;  p.lowerLinearLimit[2] = linearZLower;  p.upperLinearLimit[2] = linearZUpper;
                    p.useAngularLimits[0] = useAngularX; p.lowerAngularLimit[0] = glm::radians(angularXLower); p.upperAngularLimit[0] = glm::radians(angularXUpper);
                    p.useAngularLimits[1] = useAngularY; p.lowerAngularLimit[1] = glm::radians(angularYLower); p.upperAngularLimit[1] = glm::radians(angularYUpper);
                    p.useAngularLimits[2] = useAngularZ; p.lowerAngularLimit[2] = glm::radians(angularZLower); p.upperAngularLimit[2] = glm::radians(angularZUpper);
                    newConstraint = context.constraintCommands.createGeneric6Dof(objectA, objectB, p);
                }
                break;
            }

            if (newConstraint)
            {
                if (strlen(constraintName) > 0)
                    newConstraint->setName(std::string(constraintName));
                if (breakable)
                    newConstraint->setBreakingThreshold(breakForce, breakTorque);

                std::cout << "Created " << ConstraintTypeToStr(newConstraint->getType())
                    << " constraint" << std::endl;
                objectA = nullptr;
                objectB = nullptr;
            }
        }
        ImGui::EndDisabled();

        // Save as template
        ImGui::Separator();
        ImGui::SeparatorText("Save as Template");

        static char templateName[64] = "";
        static char templateDesc[256] = "";
        ImGui::InputText("Template Name##SaveTemplateName", templateName, IM_ARRAYSIZE(templateName));
        ImGui::InputText("Description##SaveTemplateDesc", templateDesc, IM_ARRAYSIZE(templateDesc));

        ImGui::BeginDisabled(templateName[0] == '\0');
        if (ImGui::Button("Save Current Settings as Template", ImVec2(-1, 0)))
        {
            ConstraintTemplate templ;
            templ.name = templateName;
            templ.description = templateDesc;
            templ.type = static_cast<ConstraintType>(createConstraintTypeIndex);
            templ.breakable = breakable;
            templ.breakForce = breakForce;
            templ.breakTorque = breakTorque;

            switch (createConstraintTypeIndex)
            {
            case 1:
                templ.hingeParams.pivotA = glm::vec3(hingePivot[0], hingePivot[1], hingePivot[2]);
                templ.hingeParams.axisA = glm::normalize(glm::vec3(hingeAxis[0], hingeAxis[1], hingeAxis[2]));
                templ.hingeParams.useLimits = useHingeLimits;
                templ.hingeParams.lowerLimit = glm::radians(hingeLowerLimit);
                templ.hingeParams.upperLimit = glm::radians(hingeUpperLimit);
                templ.hingeParams.useMotor = useHingeMotor;
                templ.hingeParams.motorTargetVelocity = hingeMotorVelocity;
                templ.hingeParams.motorMaxImpulse = hingeMotorForce;
                break;
            case 2:
                templ.sliderParams.useLimits = true;
                templ.sliderParams.lowerLimit = 0.0f;
                templ.sliderParams.upperLimit = sliderDistance;
                templ.sliderParams.useMotor = useSliderMotor;
                templ.sliderParams.motorTargetVelocity = sliderMotorVelocity;
                templ.sliderParams.motorMaxForce = sliderMotorForce;
                break;
            case 3:
                for (int i = 0; i < 6; ++i)
                {
                    templ.springParams.enableSpring[i] = springAxisEnabled[i];
                    templ.springParams.stiffness[i] = springStiffness;
                    templ.springParams.damping[i] = springDamping;
                }
                break;
            case 4:
                templ.dofParams.useLinearLimits[0] = useLinearX;  templ.dofParams.lowerLinearLimit[0] = linearXLower;  templ.dofParams.upperLinearLimit[0] = linearXUpper;
                templ.dofParams.useLinearLimits[1] = useLinearY;  templ.dofParams.lowerLinearLimit[1] = linearYLower;  templ.dofParams.upperLinearLimit[1] = linearYUpper;
                templ.dofParams.useLinearLimits[2] = useLinearZ;  templ.dofParams.lowerLinearLimit[2] = linearZLower;  templ.dofParams.upperLinearLimit[2] = linearZUpper;
                templ.dofParams.useAngularLimits[0] = useAngularX; templ.dofParams.lowerAngularLimit[0] = glm::radians(angularXLower); templ.dofParams.upperAngularLimit[0] = glm::radians(angularXUpper);
                templ.dofParams.useAngularLimits[1] = useAngularY; templ.dofParams.lowerAngularLimit[1] = glm::radians(angularYLower); templ.dofParams.upperAngularLimit[1] = glm::radians(angularYUpper);
                templ.dofParams.useAngularLimits[2] = useAngularZ; templ.dofParams.lowerAngularLimit[2] = glm::radians(angularZLower); templ.dofParams.upperAngularLimit[2] = glm::radians(angularZUpper);
                break;
            }

            ConstraintTemplateRegistry::getInstance().addTemplate(templ);
            ConstraintTemplateRegistry::getInstance().save();
            std::cout << "Template '" << templateName << "' saved!" << std::endl;
            templateName[0] = '\0';
            templateDesc[0] = '\0';
        }
        ImGui::EndDisabled();

        ImGui::EndTabItem();
    }

    
    //  TEMPLATES TAB
    if (ImGui::BeginTabItem("Templates"))
    {
        ImGui::TextWrapped("Load saved constraint templates and apply them to selected objects.");
        ImGui::Separator();

        auto& registry = ConstraintTemplateRegistry::getInstance();
        auto  templates = registry.getAllTemplates();

        if (templates.empty())
        {
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "No templates saved yet.");
            ImGui::Text("Create a constraint and save it as a template!");
        }
        else
        {
            ImGui::Text("Available Templates: %d", (int)templates.size());
            ImGui::Separator();

            static int selectedTemplate = 0;

            if (ImGui::BeginListBox("##Templates", ImVec2(-1, 150)))
            {
                for (int i = 0; i < (int)templates.size(); ++i)
                {
                    ImGui::PushID(i);
                    std::string label = templates[i].name + " (" +
                        ConstraintTypeToStr(templates[i].type) + ")";
                    if (ImGui::Selectable(label.c_str(), selectedTemplate == i))
                        selectedTemplate = i;
                    ImGui::PopID();
                }
                ImGui::EndListBox();
            }

            if (selectedTemplate >= 0 && selectedTemplate < (int)templates.size())
            {
                const auto& templ = templates[selectedTemplate];
                ImGui::SeparatorText("Template Details");
                ImGui::Text("Name: %s", templ.name.c_str());
                ImGui::Text("Type: %s", ConstraintTypeToStr(templ.type));
                if (!templ.description.empty())
                    ImGui::TextWrapped("Description: %s", templ.description.c_str());

                ImGui::Separator();
                ImGui::SeparatorText("Apply Template");

                if (objectA) ImGui::TextColored(ImVec4(0, 1, 0, 1), "Object A: ID %llu", objectA->getID());
                else         ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Object A: None");
                if (objectB) ImGui::TextColored(ImVec4(0, 1, 0, 1), "Object B: ID %llu", objectB->getID());
                else         ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Object B: None (Optional)");

                if (context.selectedObject && context.selectedObject->hasPhysics())
                {
                    if (ImGui::Button("Set Selected as Object A##Template")) objectA = context.selectedObject;
                    ImGui::SameLine();
                    if (ImGui::Button("Set Selected as Object B##Template")) objectB = context.selectedObject;
                }

                ImGui::Separator();
                bool canApply = objectA && objectA->hasPhysics();
                ImGui::BeginDisabled(!canApply);
                if (ImGui::Button("Apply Template to Objects", ImVec2(-1, 0)))
                {
                    auto constraint = registry.applyTemplate(templ.name, objectA, objectB);
                    if (constraint)
                    {
                        ConstraintRegistry::getInstance().addConstraint(std::move(constraint));
                        std::cout << "Applied template '" << templ.name << "'" << std::endl;
                    }
                }
                ImGui::EndDisabled();

                ImGui::Separator();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1));
                if (ImGui::Button("Delete Template", ImVec2(-1, 0)))
                {
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
    ImGui::End();
}