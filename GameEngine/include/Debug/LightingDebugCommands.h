#pragma once
#include <functional>
#include "../DirectionalLight.h"

// Commands for controlling scene lighting from debug UI
struct LightingDebugCommands
{
    // Get reference to the main directional light
    std::function<DirectionalLight& ()> getLight;
};
