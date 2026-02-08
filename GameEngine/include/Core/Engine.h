#pragma once

// Engine runtime modes
// Editor wil be UI, selection and free cursor
// Game will bw gameplay and camera capture
enum class EngineMode
{
    Editor,
    Game
}; 

// Starts up the engine and runs the main loop.
// Returns 0 on clean exit, or -1 on failure.
// Later this will move away from ints to a bool
int Start(void);