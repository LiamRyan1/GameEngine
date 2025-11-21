#pragma once
#pragma once

// Camera movement mode (Orbit or Free/WASD)
enum class CameraMode
{
    Orbit,
    Free
};

// Global camera mode variable (defined in Main.cpp)
extern CameraMode gCameraMode;

// Starts up the engine and runs the main loop.
// Returns 0 on clean exit, or -1 on failure.
// Later this will move away from ints to a bool
int Start(void);