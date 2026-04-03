#include "../include/UI/SpawnPanel.h"
#include "../External/imgui/core/imgui.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

void DrawSpawnPanel(DebugUIContext& context)
{
    ImGui::Begin("Spawn");

    static int   selectedShape = 0;
    static float spawnPos[3] = { 0.0f, 5.0f, 0.0f };
    static float mass = 1.0f;
    static bool  spawnWithPhysics = true;

    // Shape dimensions
    static float cubeSize[3] = { 1.0f, 1.0f, 1.0f };
    static float sphereRadius = 1.0f;
    static float capsuleRadius = 0.5f;
    static float capsuleHeight = 2.0f;

    // Material
    static int   selectedMaterialIndex = 0;
    static bool  useCustomMaterial = false;
    static char  customMaterialName[64] = "Custom";
    static float customFriction = 0.5f;
    static float customRestitution = 0.3f;

    // Textures
    static int  selectedTextureIndex = 0;
    static int  selectedSpecularIndex = 0;
    static int  selectedNormalIndex = 0;
    static std::vector<std::string> availableTextures;
    static bool texturesLoaded = false;
    if (!texturesLoaded && context.scene.getAvailableTextures)
    {
        availableTextures = context.scene.getAvailableTextures();
        texturesLoaded = true;
    }

    // Identity
    static char spawnName[64] = "";
    static char spawnTagInput[64] = "";
    static std::vector<std::string> spawnTags;

    //  Physics toggle
    ImGui::Checkbox("Enable Physics", &spawnWithPhysics);

    //  Shape 
    ImGui::SeparatorText("Shape");
    const char* shapes[] = { "Cube", "Sphere", "Capsule" };
    ImGui::Combo("Type", &selectedShape, shapes, IM_ARRAYSIZE(shapes));

    ImGui::SeparatorText("Dimensions");
    switch (selectedShape)
    {
    case 0: ImGui::InputFloat3("Size", cubeSize);           break;
    case 1: ImGui::InputFloat("Radius", &sphereRadius);     break;
    case 2:
        ImGui::InputFloat("Radius", &capsuleRadius);
        ImGui::InputFloat("Height", &capsuleHeight);
        break;
    }

    // Transform & Physics 
    ImGui::SeparatorText("Transform & Physics");
    ImGui::InputFloat3("Position", spawnPos);
    ImGui::InputFloat("Mass (0 = static)", &mass);
    if (mass < 0.0f) mass = 0.0f;

    // Material
    ImGui::SeparatorText("Material");
    ImGui::Checkbox("Custom Material", &useCustomMaterial);

    if (useCustomMaterial)
    {
        ImGui::InputText("Name", customMaterialName, IM_ARRAYSIZE(customMaterialName));
        ImGui::SliderFloat("Friction", &customFriction, 0.0f, 2.0f);
        ImGui::SliderFloat("Restitution", &customRestitution, 0.0f, 1.0f);
        if (ImGui::Button("Save to Presets"))
            if (context.scene.registerMaterial)
                context.scene.registerMaterial(std::string(customMaterialName),
                    customFriction, customRestitution);
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Saves this material to the registry for future use");
    }
    else
    {
        if (!context.physics.availableMaterials.empty())
        {
            std::vector<const char*> matNames;
            for (const auto& m : context.physics.availableMaterials)
                matNames.push_back(m.c_str());
            ImGui::Combo("Preset Material", &selectedMaterialIndex,
                matNames.data(), (int)matNames.size());
        }
    }

    // Textures
    ImGui::SeparatorText("Textures");
    if (!availableTextures.empty())
    {
        std::vector<const char*> texNames;
        texNames.push_back("None");
        for (const auto& t : availableTextures)
            texNames.push_back(t.c_str());

        ImGui::Combo("Diffuse Texture", &selectedTextureIndex,
            texNames.data(), (int)texNames.size());
        ImGui::Combo("Specular Map", &selectedSpecularIndex,
            texNames.data(), (int)texNames.size());
        ImGui::Combo("Normal Map", &selectedNormalIndex,
            texNames.data(), (int)texNames.size());
    }
    else
    {
        ImGui::TextDisabled("No textures found");
    }

    // Identity
    ImGui::Separator();
    ImGui::SeparatorText("Identity");
    ImGui::InputText("Name (Optional)", spawnName, IM_ARRAYSIZE(spawnName));
    ImGui::TextDisabled("Leave blank for auto-generated name (e.g. Cube_3)");
    ImGui::Spacing();

    ImGui::Text("Tags:");
    for (int i = 0; i < (int)spawnTags.size(); ++i)
    {
        ImGui::SameLine();
        ImGui::PushID(i);
        std::string chip = spawnTags[i] + " x";
        if (ImGui::SmallButton(chip.c_str()))
            spawnTags.erase(spawnTags.begin() + i);
        ImGui::PopID();
    }
    ImGui::SetNextItemWidth(140.0f);
    ImGui::InputText("##NewTag", spawnTagInput, IM_ARRAYSIZE(spawnTagInput));
    ImGui::SameLine();
    if (ImGui::Button("Add Tag") && spawnTagInput[0] != '\0')
    {
        std::string t(spawnTagInput);
        if (std::find(spawnTags.begin(), spawnTags.end(), t) == spawnTags.end())
            spawnTags.push_back(t);
        spawnTagInput[0] = '\0';
    }
    ImGui::TextDisabled("e.g. enemy, player, interactable");

    //  Spawn button 
    ImGui::Separator();
    if (ImGui::Button("Spawn Object", ImVec2(-1, 0)) && context.scene.spawnObject)
    {
        ShapeType shapeType;
        glm::vec3 size;

        switch (selectedShape)
        {
        case 0: shapeType = ShapeType::CUBE;
            size = glm::vec3(cubeSize[0], cubeSize[1], cubeSize[2]);    break;
        case 1: shapeType = ShapeType::SPHERE;
            size = glm::vec3(sphereRadius);                              break;
        case 2: shapeType = ShapeType::CAPSULE;
            size = glm::vec3(capsuleRadius, capsuleHeight, capsuleRadius); break;
        default:shapeType = ShapeType::CUBE; size = glm::vec3(1.0f);
        }

        std::string materialName;
        if (useCustomMaterial)
        {
            materialName = std::string(customMaterialName);
            if (context.scene.registerMaterial)
                context.scene.registerMaterial(materialName,
                    customFriction, customRestitution);
        }
        else
        {
            materialName =
                (selectedMaterialIndex >= 0 &&
                    selectedMaterialIndex < (int)context.physics.availableMaterials.size())
                ? context.physics.availableMaterials[selectedMaterialIndex]
                : "Default";
        }

        auto texPath = [&](int idx) -> std::string {
            return (idx > 0 && idx <= (int)availableTextures.size())
                ? availableTextures[idx - 1] : "";
            };

        std::string texturePath = texPath(selectedTextureIndex);
        std::string specularPath = texPath(selectedSpecularIndex);
        std::string normalPath = texPath(selectedNormalIndex);

        GameObject* spawned = nullptr;
        if (spawnWithPhysics)
            spawned = context.scene.spawnObject(
                shapeType,
                glm::vec3(spawnPos[0], spawnPos[1], spawnPos[2]),
                size, mass, materialName, texturePath, specularPath);
        else
            spawned = context.scene.spawnRenderObject(
                shapeType,
                glm::vec3(spawnPos[0], spawnPos[1], spawnPos[2]),
                size, texturePath, specularPath);

        if (spawned)
        {
            if (spawnName[0] != '\0')
            {
                spawned->setName(std::string(spawnName));
            }
            else
            {
                const char* sn =
                    selectedShape == 0 ? "Cube" :
                    selectedShape == 1 ? "Sphere" : "Capsule";
                spawned->setName(std::string(sn) + "_" +
                    std::to_string(spawned->getID()));
            }

            for (const auto& tag : spawnTags)
                spawned->addTag(tag);

            if (!normalPath.empty())
                spawned->getRender().setNormalTexturePath(normalPath);

            spawnName[0] = '\0';
        }
    }

    ImGui::End();
}