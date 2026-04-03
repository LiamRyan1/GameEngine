#include "../include/UI/ModelImporterPanel.h"
#include "../External/imgui/core/imgui.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

void DrawModelImporterPanel(DebugUIContext& context)
{
    ImGui::Begin("Model Importer");

    static int  selectedModelIndex = 0;
    static std::vector<std::string> availableModels;
    static bool modelsLoaded = false;
    if (!modelsLoaded && context.scene.getAvailableModels)
    {
        availableModels = context.scene.getAvailableModels();
        modelsLoaded = true;
    }

    static float modelPos[3] = { 0.0f, 2.0f, 0.0f };
    static float modelScale[3] = { 1.0f, 1.0f, 1.0f };
    static bool  enablePhysics = false;
    static float modelMass = 1.0f;
    static float physicsBoxScale[3] = { 0.9f, 0.9f, 0.9f };
    static int   selectedMatIndex = 0;

    ImGui::Text("Load .obj Model");
    ImGui::Separator();

    // Model dropdown
    if (!availableModels.empty())
    {
        std::vector<const char*> names;
        for (const auto& m : availableModels) names.push_back(m.c_str());
        ImGui::Combo("Model File", &selectedModelIndex,
            names.data(), (int)names.size());
    }
    else
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No .obj files found in models/");
    }

    ImGui::InputFloat3("Position", modelPos);
    ImGui::InputFloat3("Scale", modelScale);

    //Physics 
    ImGui::Separator();
    ImGui::SeparatorText("Physics (Optional)");
    ImGui::Checkbox("Enable Physics", &enablePhysics);

    if (enablePhysics)
    {
        ImGui::InputFloat("Mass (0 = static)", &modelMass);
        if (modelMass < 0.0f) modelMass = 0.0f;

        ImGui::TextWrapped("Collision box calculated from model bounds");
        ImGui::SliderFloat3("Box Scale Factor", physicsBoxScale, 0.5f, 1.5f);
        ImGui::TextDisabled("1.0 = exact fit, <1.0 = smaller, >1.0 = larger");

        if (!context.physics.availableMaterials.empty())
        {
            std::vector<const char*> matNames;
            for (const auto& m : context.physics.availableMaterials)
                matNames.push_back(m.c_str());
            ImGui::Combo("Material", &selectedMatIndex,
                matNames.data(), (int)matNames.size());
        }
    }

    // Spawn button 
    ImGui::Separator();
    if (ImGui::Button("Load & Spawn Model", ImVec2(-1, 0)))
    {
        if (context.scene.loadAndSpawnModel &&
            selectedModelIndex >= 0 &&
            selectedModelIndex < (int)availableModels.size())
        {
            std::string mat = "Default";
            if (enablePhysics &&
                selectedMatIndex >= 0 &&
                selectedMatIndex < (int)context.physics.availableMaterials.size())
                mat = context.physics.availableMaterials[selectedMatIndex];

            GameObject* obj = context.scene.loadAndSpawnModel(
                availableModels[selectedModelIndex],
                glm::vec3(modelPos[0], modelPos[1], modelPos[2]),
                glm::vec3(modelScale[0], modelScale[1], modelScale[2]),
                enablePhysics,
                modelMass,
                glm::vec3(physicsBoxScale[0], physicsBoxScale[1], physicsBoxScale[2]),
                mat);

            std::cout << (obj ? "Model loaded successfully!" : "Failed to load model.")
                << std::endl;
        }
    }

    ImGui::Separator();
    ImGui::TextWrapped("Place .obj files in the models/ folder. Restart to refresh list.");

    ImGui::End();
}