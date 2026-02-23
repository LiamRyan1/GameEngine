#include "../include/UI/TriggerEditorPanel.h"
#include "../include/Physics/Trigger.h"
#include "../include/Physics/TriggerRegistry.h"
#include "../External/imgui/core/imgui.h"
#include <algorithm>
#include <iostream>

static const char* TriggerTypeToString(TriggerType type)
{
    switch (type)
    {
    case TriggerType::GOAL_ZONE:  return "Goal Zone";
    case TriggerType::DEATH_ZONE: return "Death Zone";
    case TriggerType::CHECKPOINT: return "Checkpoint";
    case TriggerType::TELEPORT:   return "Teleport";
    case TriggerType::SPEED_ZONE: return "Speed Zone";
    case TriggerType::CUSTOM:     return "Custom";
    default:                      return "Unknown";
    }
}

static TriggerType IndexToTriggerType(int index)
{
    switch (index)
    {
    case 0: return TriggerType::GOAL_ZONE;
    case 1: return TriggerType::DEATH_ZONE;
    case 2: return TriggerType::CHECKPOINT;
    case 3: return TriggerType::TELEPORT;
    case 4: return TriggerType::SPEED_ZONE;
    case 5: return TriggerType::CUSTOM;
    default: return TriggerType::CUSTOM;
    }
}

void DrawTriggerEditorPanel(DebugUIContext& context)
{
    ImGui::Begin("Trigger Editor");

    // All persistent UI state as statics - same pattern as other panels
    static Trigger* selectedTrigger = nullptr;
    static char     newTriggerName[128] = "Trigger_1";
    static int      selectedTypeIndex = 0;
    static float    newPosition[3] = { 0.0f, 2.0f, 0.0f };
    static float    newSize[3] = { 2.0f, 2.0f, 2.0f };
    static float    teleportDest[3] = { 0.0f, 0.0f, 0.0f };
    static float    forceDir[3] = { 0.0f, 1.0f, 0.0f };
    static float    forceMag = 10.0f;

    // Refresh list every frame
    const std::vector<Trigger*> allTriggers = TriggerRegistry::getInstance().getAllTriggers();

    // Validate selectedTrigger is still alive
    if (selectedTrigger)
    {
        bool stillValid = std::find(allTriggers.begin(), allTriggers.end(),
            selectedTrigger) != allTriggers.end();
        if (!stillValid)
            selectedTrigger = nullptr;
    }

    if (ImGui::BeginTabBar("TriggerEditorTabs"))
    {
        // ================= CREATE TAB =================
        if (ImGui::BeginTabItem("Create"))
        {
            ImGui::SeparatorText("New Trigger");
            ImGui::InputText("Name", newTriggerName, sizeof(newTriggerName));

            const char* triggerTypes[] =
            { "Goal Zone", "Death Zone", "Checkpoint", "Teleport", "Speed Zone", "Custom" };
            ImGui::Combo("Type", &selectedTypeIndex, triggerTypes, IM_ARRAYSIZE(triggerTypes));

            ImGui::Separator();
            ImGui::TextWrapped("Description:");
            switch (selectedTypeIndex)
            {
            case 0: ImGui::TextWrapped("Triggers win condition when player enters."); break;
            case 1: ImGui::TextWrapped("Kills or respawns objects that enter."); break;
            case 2: ImGui::TextWrapped("Saves player progress when entered."); break;
            case 3: ImGui::TextWrapped("Instantly moves objects to a destination."); break;
            case 4: ImGui::TextWrapped("Applies force to objects entering."); break;
            case 5: ImGui::TextWrapped("User-defined behavior via callbacks."); break;
            }

            ImGui::Separator();
            ImGui::DragFloat3("Position", newPosition, 0.1f);
            ImGui::DragFloat3("Size (Half Extents)", newSize, 0.1f, 0.1f, 100.0f);

            ImGui::Separator();
            ImGui::SeparatorText("Type-Specific Settings");

            TriggerType selectedType = IndexToTriggerType(selectedTypeIndex);
            if (selectedType == TriggerType::TELEPORT)
            {
                ImGui::DragFloat3("Teleport Destination", teleportDest, 0.1f);
            }
            else if (selectedType == TriggerType::SPEED_ZONE)
            {
                ImGui::DragFloat3("Force Direction", forceDir, 0.01f, -1.0f, 1.0f);
                ImGui::DragFloat("Force Magnitude", &forceMag, 0.5f, 0.0f, 1000.0f);
            }
            else
            {
                ImGui::TextDisabled("No additional parameters for this type");
            }

            ImGui::Separator();

            if (ImGui::Button("Create Trigger", ImVec2(-1, 0)))
            {
                if (context.triggerCommands.createTrigger)
                {
                    Trigger* newTrigger = context.triggerCommands.createTrigger(
                        newTriggerName,
                        selectedType,
                        glm::vec3(newPosition[0], newPosition[1], newPosition[2]),
                        glm::vec3(newSize[0], newSize[1], newSize[2]));

                    if (newTrigger)
                    {
                        if (selectedType == TriggerType::TELEPORT)
                            context.triggerCommands.setTeleportDestination(newTrigger,
                                glm::vec3(teleportDest[0], teleportDest[1], teleportDest[2]));
                        else if (selectedType == TriggerType::SPEED_ZONE)
                            context.triggerCommands.setForce(newTrigger,
                                glm::vec3(forceDir[0], forceDir[1], forceDir[2]), forceMag);

                        selectedTrigger = newTrigger;

                        // Reset fields
                        strcpy(newTriggerName, "Trigger_1");
                        newPosition[0] = 0.0f; newPosition[1] = 2.0f; newPosition[2] = 0.0f;
                        newSize[0] = 2.0f;     newSize[1] = 2.0f;     newSize[2] = 2.0f;
                    }
                }
            }

            ImGui::EndTabItem();
        }

        // ================= LIST TAB =================
        if (ImGui::BeginTabItem("List"))
        {
            if (allTriggers.empty())
            {
                ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "No triggers in scene");
            }
            else
            {
                ImGui::Text("Total triggers: %zu", allTriggers.size());
                ImGui::Separator();

                for (Trigger* trigger : allTriggers)
                {
                    if (!trigger) continue;

                    bool isSelected = (selectedTrigger == trigger);
                    ImGui::PushID(static_cast<int>(trigger->getID()));

                    if (ImGui::Selectable(trigger->getName().c_str(), isSelected))
                        selectedTrigger = trigger;

                    ImGui::PopID();
                }
            }

            ImGui::EndTabItem();
        }

        // ================= PROPERTIES TAB =================
        if (ImGui::BeginTabItem("Properties"))
        {
            if (selectedTrigger)
            {
                Trigger* trigger = selectedTrigger;

                ImGui::SeparatorText("Trigger Properties");
                ImGui::Text("ID: %llu", static_cast<unsigned long long>(trigger->getID()));
                ImGui::Text("Type: %s", TriggerTypeToString(trigger->getType()));

                ImGui::Separator();

                // Editable name - sync buffer when selection changes
                static char    nameBuffer[128] = {};
                static Trigger* lastEdited = nullptr;
                if (lastEdited != trigger)
                {
                    strncpy(nameBuffer, trigger->getName().c_str(), sizeof(nameBuffer) - 1);
                    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
                    lastEdited = trigger;
                }
                if (ImGui::InputText("Name##edit", nameBuffer, sizeof(nameBuffer),
                    ImGuiInputTextFlags_EnterReturnsTrue))
                    trigger->setName(nameBuffer);
                ImGui::SameLine();
                ImGui::TextDisabled("(press Enter)");

                ImGui::Separator();

                glm::vec3 pos = trigger->getPosition();
                if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
                    trigger->setPosition(pos);

                glm::vec3 size = trigger->getSize();
                if (ImGui::DragFloat3("Size (Half Extents)", &size.x, 0.1f, 0.1f, 100.0f))
                    trigger->setSize(size);

                ImGui::Separator();

                bool enabled = trigger->isEnabled();
                if (ImGui::Checkbox("Enabled", &enabled))
                    trigger->setEnabled(enabled);

                ImGui::Separator();
                ImGui::SeparatorText("Type-Specific");

                if (trigger->getType() == TriggerType::TELEPORT)
                {
                    glm::vec3 dest = trigger->getTeleportDestination();
                    if (ImGui::DragFloat3("Destination", &dest.x, 0.1f))
                        trigger->setTeleportDestination(dest);
                }
                else if (trigger->getType() == TriggerType::SPEED_ZONE)
                {
                    glm::vec3 dir = trigger->getForceDirection();
                    float mag = trigger->getForceMagnitude();
                    bool changed = false;
                    changed |= ImGui::DragFloat3("Force Direction", &dir.x, 0.01f, -1.0f, 1.0f);
                    changed |= ImGui::DragFloat("Force Magnitude", &mag, 0.5f, 0.0f, 1000.0f);
                    if (changed)
                        trigger->setForce(dir, mag);
                }
                else
                {
                    ImGui::TextDisabled("No additional parameters for this type");
                }

                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

                if (ImGui::Button("Delete Trigger", ImVec2(-1, 0)))
                {
                    if (context.triggerCommands.removeTrigger)
                    {
                        context.triggerCommands.removeTrigger(trigger);
                        selectedTrigger = nullptr;
                    }
                }

                ImGui::PopStyleColor(3);
            }
            else
            {
                ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "No trigger selected");
                ImGui::TextDisabled("Select a trigger from the List tab.");
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}