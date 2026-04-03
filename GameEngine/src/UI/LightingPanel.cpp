#include "../include/UI/LightingPanel.h"
#include "../include/Rendering/DirectionalLight.h"
#include "../External/imgui/core/imgui.h"

void DrawLightingPanel(DebugUIContext& context)
{
    ImGui::Begin("Lighting");

    if (!context.lighting.getLight)
    {
        ImGui::TextDisabled("No light available");
        ImGui::End();
        return;
    }

    DirectionalLight& light = context.lighting.getLight();

    // Direction 
    glm::vec3 dir = light.getDirection();
    float dirArr[3] = { dir.x, dir.y, dir.z };
    if (ImGui::DragFloat3("Direction", dirArr, 0.01f, -1.0f, 1.0f))
        light.setDirection(glm::vec3(dirArr[0], dirArr[1], dirArr[2]));

    // Color 
    glm::vec3 col = light.getColor();
    float colArr[3] = { col.r, col.g, col.b };
    if (ImGui::ColorEdit3("Color", colArr))
        light.setColor(glm::vec3(colArr[0], colArr[1], colArr[2]));

    // Intensity
    float intensity = light.getIntensity();
    if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 5.0f))
        light.setIntensity(intensity);

    // Presets 
    ImGui::Separator();
    ImGui::Text("Presets:");

    if (ImGui::Button("Noon Sun"))
    {
        light.setDirection(glm::vec3(0.0f, -1.0f, 0.0f));
        light.setColor(glm::vec3(1.0f, 1.0f, 0.9f));
        light.setIntensity(1.0f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Sunset"))
    {
        light.setDirection(glm::vec3(1.0f, -0.3f, 0.0f));
        light.setColor(glm::vec3(1.0f, 0.6f, 0.3f));
        light.setIntensity(0.8f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Night"))
    {
        light.setDirection(glm::vec3(0.2f, -1.0f, 0.3f));
        light.setColor(glm::vec3(0.3f, 0.3f, 0.5f));
        light.setIntensity(0.3f);
    }

    ImGui::End();
}