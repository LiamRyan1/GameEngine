#include "../include/Debug/DebugUI.h"
#include "imgui.h"
#include <vector>
#include <string>
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

    // Shape-specific parameters
	static float cubeSize[3] = { 1.0f, 1.0f, 1.0f }; // cubes use half-extents
    static float sphereRadius = 1.0f; 
	static float capsuleRadius = 0.5f; // capsules use radius + height
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
					size = glm::vec3(capsuleRadius, capsuleHeight, 0.0f);
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
            context.scene.spawnObject(
                shapeType,
                glm::vec3(spawnPos[0], spawnPos[1], spawnPos[2]),
                size,
                mass,
                materialName,
                texturePath
            );
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

    // Nothing selected → show hint and exit early.
    // Prevents null pointer access and keeps UI clear.
    if (!context.selectedObject)
    {
        ImGui::Text("No object selected.");
        ImGui::Text("Click an object in the scene to inspect it.");
        ImGui::End();
        return;
    }

    // Pull current transform from the selected object.
    // We copy into local values because ImGui works with plain floats.
    glm::vec3 pos = context.selectedObject->getPosition();
    glm::vec3 scale = context.selectedObject->getScale();

    ImGui::Text("Selected Object");
    ImGui::Separator();

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
    // Scale
    // ----------------------------
    // Same approach as position, but clamp scale to avoid negatives/zero.
    float scaleArr[3] = { scale.x, scale.y, scale.z };

    if (ImGui::DragFloat3("Scale", scaleArr, 0.05f, 0.01f, 1000.0f))
    {
        context.selectedObject->setScale(glm::vec3(scaleArr[0], scaleArr[1], scaleArr[2]));
    }

    ImGui::End();
}
