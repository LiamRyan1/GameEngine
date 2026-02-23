#pragma once

#include "TimeDebugView.h"
#include "PhysicsDebugView.h"
#include "SceneDebugCommands.h"
#include "LightingDebugCommands.h"
#include "ConstraintDebugView.h"
#include "ConstraintDebugCommands.h"
#include "TriggerDebugView.h"       
#include "TriggerDebugCommands.h"  
#include "Scene/GameObject.h"

struct DebugUIContext
{
    TimeDebugView time;
    PhysicsDebugView physics;
    SceneDebugCommands scene;
    LightingDebugCommands lighting;
    ConstraintDebugView constraints;
    ConstraintDebugCommands constraintCommands;
    TriggerDebugView triggers;          
    TriggerDebugCommands triggerCommands;
    // Currently selected object (set by Engine picking).
    // DebugUI may display/edit it, but does not own it.
    GameObject* selectedObject = nullptr;

};
