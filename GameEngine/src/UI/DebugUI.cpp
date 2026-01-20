#include "../include/Debug/DebugUI.h"
#include "imgui.h"

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
    static float spawnPos[3] = { 0.0f, 5.0f, 0.0f };
    static bool withPhysics = true;

    // Position input for new object
    ImGui::InputFloat3("Position", spawnPos);

    // Whether the spawned cube should have a physics body
    ImGui::Checkbox("With Physics", &withPhysics);

    // Request object creation when button is pressed
    if (ImGui::Button("Spawn Cube"))
    {
        // Only issue the request if the Engine has provided a valid command
        if (context.scene.spawnCube)
        {
            context.scene.spawnCube(
                glm::vec3(spawnPos[0], spawnPos[1], spawnPos[2]),
                withPhysics
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
