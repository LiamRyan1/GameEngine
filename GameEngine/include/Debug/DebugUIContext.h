#pragma once

#include "TimeDebugView.h"
#include "PhysicsDebugView.h"
#include "SceneDebugCommands.h"

struct DebugUIContext
{
    TimeDebugView time;
    PhysicsDebugView physics;
    SceneDebugCommands scene;
};