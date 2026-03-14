#include "../include/UI/ForceGeneratorPanel.h"
#include "../include/Debug/DebugUI.h"
#include "../include/Physics/ForceGeneratorRegistry.h"
#include "../include/Physics/ForceGenerator.h"
#include "../External/imgui/core/imgui.h"
#include <iostream>
#include <string>


void DrawForceGeneratorPanel(DebugUIContext& context)
{
    ImGui::Begin("Force Generators");

    auto& reg = ForceGeneratorRegistry::getInstance();
    auto generators = reg.getAllGenerators();

    // ── Active Generators ─────────────────────────────────────────────────────
    ImGui::SeparatorText("Active Generators");

    if (generators.empty())
    {
        ImGui::TextDisabled("No active force generators");
    }
    else
    {
        ImGui::Text("Count: %d", (int)generators.size());
        ImGui::Separator();

        for (int i = 0; i < (int)generators.size(); ++i)
        {
            ForceGenerator* gen = generators[i];
            if (!gen) continue;

            ImGui::PushID(i);

            const char* typeStr = "Unknown";
            switch (gen->getType())
            {
            case ForceGeneratorType::WIND:         typeStr = "[Wind]";         break;
            case ForceGeneratorType::GRAVITY_WELL: typeStr = "[Gravity Well]"; break;
            case ForceGeneratorType::VORTEX:       typeStr = "[Vortex]";       break;
            case ForceGeneratorType::EXPLOSION:    typeStr = "[Explosion]";    break;
            }

            std::string header = std::string(typeStr) + " " + gen->getName();

            if (ImGui::CollapsingHeader(header.c_str()))
            {
                // ── Shared controls ───────────────────────────────────────────

                bool enabled = gen->isEnabled();
                if (ImGui::Checkbox("Enabled", &enabled))
                    gen->setEnabled(enabled);

                glm::vec3 pos = gen->getPosition();
                float posArr[3] = { pos.x, pos.y, pos.z };
                if (ImGui::DragFloat3("Position", posArr, 0.1f))
                    gen->setPosition(glm::vec3(posArr[0], posArr[1], posArr[2]));

                float radius = gen->getRadius();
                if (ImGui::DragFloat("Radius (0 = infinite)", &radius, 0.1f, 0.0f, 500.0f))
                    gen->setRadius(radius);

                float strength = gen->getStrength();
                if (ImGui::DragFloat("Strength", &strength, 0.5f, -5000.0f, 5000.0f))
                    gen->setStrength(strength);

                // ── Type-specific controls ────────────────────────────────────

                switch (gen->getType())
                {
                case ForceGeneratorType::WIND:
                {
                    auto* w = static_cast<WindGenerator*>(gen);
                    glm::vec3 dir = w->getDirection();
                    float dirArr[3] = { dir.x, dir.y, dir.z };
                    if (ImGui::DragFloat3("Direction", dirArr, 0.01f, -1.0f, 1.0f))
                        w->setDirection(glm::vec3(dirArr[0], dirArr[1], dirArr[2]));
                    break;
                }
                case ForceGeneratorType::GRAVITY_WELL:
                {
                    auto* g = static_cast<GravityWellGenerator*>(gen);
                    float minDist = g->getMinDistance();
                    if (ImGui::DragFloat("Min Distance", &minDist, 0.05f, 0.01f, 10.0f))
                        g->setMinDistance(minDist);
                    ImGui::TextDisabled("Clamps distance to avoid infinite force at center");
                    break;
                }
                case ForceGeneratorType::VORTEX:
                {
                    auto* v = static_cast<VortexGenerator*>(gen);

                    glm::vec3 axis = v->getAxis();
                    float axisArr[3] = { axis.x, axis.y, axis.z };
                    if (ImGui::DragFloat3("Axis", axisArr, 0.01f, -1.0f, 1.0f))
                        v->setAxis(glm::vec3(axisArr[0], axisArr[1], axisArr[2]));

                    float pull = v->getPullStrength();
                    if (ImGui::DragFloat("Pull Strength", &pull, 0.5f, -1000.0f, 1000.0f))
                        v->setPullStrength(pull);
                    break;
                }
                case ForceGeneratorType::EXPLOSION:
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                        "One-shot — will be removed after firing");
                    break;

                default: break;
                }

                // ── Remove button ─────────────────────────────────────────────
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
                if (ImGui::Button("Remove##ForceGenRemove", ImVec2(-1, 0)))
                    reg.removeGenerator(gen->getName());
                ImGui::PopStyleColor(3);
            }

            ImGui::PopID();
        }
    }

    // ── Create New Generator ──────────────────────────────────────────────────
    ImGui::Spacing();
    ImGui::SeparatorText("Create Generator");

    static int   selectedType = 0;
    static char  genName[64] = "ForceGen";
    static float genPos[3] = { 0.0f, 0.0f, 0.0f };
    static float genRadius = 20.0f;
    static float genStrength = 100.0f;

    static float windDir[3] = { 1.0f, 0.0f, 0.0f };

    static float gravMinDist = 1.0f;

    static float vortexAxis[3] = { 0.0f, 1.0f, 0.0f };
    static float vortexPull = 50.0f;

    static float explosionRadius = 20.0f;
    static float explosionStr = 2000.0f;

    const char* typeNames[] = { "Wind", "Gravity Well", "Vortex", "Explosion" };
    ImGui::Combo("Type##CreateForceType", &selectedType, typeNames, IM_ARRAYSIZE(typeNames));
    ImGui::InputText("Name##ForceGenName", genName, IM_ARRAYSIZE(genName));
    ImGui::DragFloat3("Position##ForceGenPos", genPos, 0.5f);

    if (selectedType != 3)
    {
        ImGui::DragFloat("Radius (0 = infinite)##ForceGenRadius", &genRadius, 0.5f, 0.0f, 500.0f);
        ImGui::DragFloat("Strength##ForceGenStr", &genStrength, 1.0f, -5000.0f, 5000.0f);
    }

    switch (selectedType)
    {
    case 0:
        ImGui::DragFloat3("Direction##ForceWindDir", windDir, 0.01f, -1.0f, 1.0f);
        ImGui::TextDisabled("Direction will be normalized on creation");
        break;

    case 1:
        ImGui::DragFloat("Min Distance##GravMinDist", &gravMinDist, 0.05f, 0.01f, 10.0f);
        ImGui::TextDisabled("Positive strength = attract, negative = repel");
        break;

    case 2:
        ImGui::DragFloat3("Axis##ForceVortexAxis", vortexAxis, 0.01f, -1.0f, 1.0f);
        ImGui::DragFloat("Pull Strength##VortexPull", &vortexPull, 1.0f, -1000.0f, 1000.0f);
        ImGui::TextDisabled("Axis will be normalized on creation");
        break;

    case 3:
        ImGui::DragFloat("Radius##ExplosionRadius", &explosionRadius, 0.5f, 1.0f, 500.0f);
        ImGui::DragFloat("Strength##ExplosionStr", &explosionStr, 10.0f, 0.0f, 50000.0f);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            "One-shot — fires immediately and is removed the same frame");
        break;
    }

    ImGui::Spacing();

    bool nameProvided = genName[0] != '\0';
    ImGui::BeginDisabled(!nameProvided);

    if (ImGui::Button("Create##ForceGenCreate", ImVec2(-1, 0)))
    {
        glm::vec3 p(genPos[0], genPos[1], genPos[2]);
        std::string n(genName);

        switch (selectedType)
        {
        case 0:
            reg.createWind(n, p, genRadius,
                glm::vec3(windDir[0], windDir[1], windDir[2]),
                genStrength);
            break;

        case 1:
            reg.addGenerator(std::make_unique<GravityWellGenerator>(
                n, p, genRadius, genStrength, gravMinDist));
            break;

        case 2:
            reg.createVortex(n, p, genRadius,
                glm::vec3(vortexAxis[0], vortexAxis[1], vortexAxis[2]),
                genStrength, vortexPull);
            break;

        case 3:
            reg.createExplosion(n, p, explosionRadius, explosionStr);
            break;
        }

        std::cout << "[ForceGeneratorPanel] Created '" << n << "'" << std::endl;
    }

    ImGui::EndDisabled();

    if (!nameProvided)
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Name required");

    ImGui::End();
}