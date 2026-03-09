#pragma once

struct DebugUIContext;

/**
 * @brief Draws the Force Generator editor panel.
 *
 * Provides a self-contained ImGui panel for:
 *   - Listing and live-editing all active force generators (Wind, Gravity Well, Vortex)
 *   - Creating new generators of any type with full parameter control
 *   - Firing one-shot explosions directly from the editor
 *   - Removing individual generators by name
 *
 * Matches the save/load format used by Scene::saveToFile() / Scene::loadFromFile()
 * so any generator created here will round-trip correctly through JSON.
 *
 * Call DrawForceGeneratorPanel(context) once per frame inside DebugUI::draw(),
 * alongside DrawTriggerEditorPanel(context).
 */

void DrawForceGeneratorPanel(DebugUIContext& context);