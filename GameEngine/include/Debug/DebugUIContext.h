#pragma once

#include "TimeDebugView.h"
#include "PhysicsDebugView.h"
#include "SceneDebugCommands.h"
#include "GameObject.h"

struct DebugUIContext
{
    TimeDebugView time;
    PhysicsDebugView physics;
    SceneDebugCommands scene;

    // Currently selected object (set by Engine picking).
    // DebugUI may display/edit it, but does not own it.
    GameObject* selectedObject = nullptr;
};