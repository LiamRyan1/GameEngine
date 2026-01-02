#pragma once

#include "DebugUIContext.h"

// Draws all debug/editor UI using ImGui.
// Reads engine state from DebugUIContext and issues debug commands.
// Does NOT own rendering or engine systems.

class DebugUI
{
public:
    void draw(const DebugUIContext& context);
};