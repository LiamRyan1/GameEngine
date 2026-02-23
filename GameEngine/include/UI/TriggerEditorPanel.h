#pragma once

#include "Debug/DebugUIContext.h"

/**
 * @brief Draws the Trigger Editor panel
 *
 *  renders an ImGui window that allows users to:
 * - Create new triggers with various types
 * - View and edit existing triggers
 * - Delete triggers
 * - Configure trigger-specific parameters (teleport destinations, force directions, etc.)
 *
 * @param context Debug UI context containing trigger view state and commands
 */
void DrawTriggerEditorPanel(DebugUIContext& context);