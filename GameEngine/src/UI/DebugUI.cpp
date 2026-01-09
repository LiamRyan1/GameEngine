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
}
