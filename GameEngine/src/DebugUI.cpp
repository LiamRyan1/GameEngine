#include "../include/Debug/DebugUI.h"
#include "imgui.h"

void DebugUI::draw(const DebugUIContext& context)
{
    ImGui::Begin("Stats");

    ImGui::Text("FPS: %.1f", context.time.fps);
    ImGui::Text("Delta Time: %.4f s", context.time.deltaTime);

    ImGui::Separator();

    ImGui::Text("Rigid Bodies: %d", context.physics.rigidBodyCount);
    ImGui::Text("Physics Enabled: %s", context.physics.physicsEnabled ? "Yes" : "No");

    ImGui::End();
}
