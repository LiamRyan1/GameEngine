#include "../include/UI/PointLightPanel.h"
#include "../include/Rendering/PointLightRegistry.h"
#include "../include/Rendering/PointLight.h"
#include "../External/imgui/core/imgui.h"
#include <iostream>

void DrawPointLightPanel(DebugUIContext& context)
{
    ImGui::Begin("Point Lights");

    auto& reg = PointLightRegistry::getInstance();
    auto lights = reg.getAllLights();

    // Active Lights
    ImGui::SeparatorText("Active Lights");

    if (lights.empty())
    {
        ImGui::TextDisabled("No active point lights");
    }
    else
    {
        ImGui::Text("Count: %d", (int)lights.size());
        ImGui::Separator();

        for (int i = 0; i < (int)lights.size(); ++i)
        {
            PointLight* light = lights[i];
            if (!light) continue;

            ImGui::PushID(i);

            std::string header = "[Light] " + light->getName();

            if (ImGui::CollapsingHeader(header.c_str()))
            {
                bool enabled = light->isEnabled();
                if (ImGui::Checkbox("Enabled", &enabled))
                    light->setEnabled(enabled);

                glm::vec3 pos = light->getPosition();
                float posArr[3] = { pos.x, pos.y, pos.z };
                if (ImGui::DragFloat3("Position", posArr, 0.1f))
                    light->setPosition(glm::vec3(posArr[0], posArr[1], posArr[2]));

                glm::vec3 col = light->getColour();
                float colArr[3] = { col.r, col.g, col.b };
                if (ImGui::ColorEdit3("Colour", colArr))
                    light->setColour(glm::vec3(colArr[0], colArr[1], colArr[2]));

                float intensity = light->getIntensity();
                if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 10.0f))
                    light->setIntensity(intensity);

                float radius = light->getRadius();
                if (ImGui::DragFloat("Radius", &radius, 0.5f, 0.1f, 200.0f))
                    light->setRadius(radius);

                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
                if (ImGui::Button("Remove##LightRemove", ImVec2(-1, 0)))
                    reg.removeLight(light->getName());
                ImGui::PopStyleColor(3);
            }

            ImGui::PopID();
        }
    }

    // Create New Light
    ImGui::Spacing();
    ImGui::SeparatorText("Create Light");

    static char  lightName[64] = "PointLight";
    static float lightPos[3] = { 0.0f, 3.0f, 0.0f };
    static float lightCol[3] = { 1.0f, 1.0f, 1.0f };
    static float lightIntensity = 1.0f;
    static float lightRadius = 15.0f;

    ImGui::InputText("Name##LightName", lightName, IM_ARRAYSIZE(lightName));
    ImGui::DragFloat3("Position##LightPos", lightPos, 0.5f);
    ImGui::ColorEdit3("Colour##LightCol", lightCol);
    ImGui::DragFloat("Intensity##LightIntensity", &lightIntensity, 0.05f, 0.0f, 10.0f);
    ImGui::DragFloat("Radius##LightRadius", &lightRadius, 0.5f, 0.1f, 200.0f);

    ImGui::Spacing();

    bool nameProvided = lightName[0] != '\0';
    ImGui::BeginDisabled(!nameProvided);

    if (ImGui::Button("Create##LightCreate", ImVec2(-1, 0)))
    {
        reg.addLight(
            std::string(lightName),
            glm::vec3(lightPos[0], lightPos[1], lightPos[2]),
            glm::vec3(lightCol[0], lightCol[1], lightCol[2]),
            lightIntensity,
            lightRadius
        );
        std::cout << "[PointLightPanel] Created '" << lightName << "'" << std::endl;
    }

    ImGui::EndDisabled();

    if (!nameProvided)
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Name required");

    ImGui::End();
}