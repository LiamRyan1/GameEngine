#include "../include/Input.h"

// keysDown[key]     → true while key is being held
// keysPressed[key]  → true for ONE frame when key transitions RELEASED → PRESSED
// keysReleased[key] → true for ONE frame when key transitions PRESSED → RELEASED
// They are reset every frame for pressed/released transitions.
// 1024 stores key states
static bool keysDown[1024] = { false };
static bool keysPressed[1024] = { false };
static bool keysReleased[1024] = { false };

// GLFW window pointer, stored so the input module knows which window it belongs to.
// It’s the Input system’s saved reference to the game window.
// Without it, we can’t read input from GLFW.
static GLFWwindow* internalWindow = nullptr;

// KeyCallback
// This is called directly by GLFW every time a key's state changes.
// We only update states here; the engine will read them later.
// GLFW calls KeyCallback -> Update the key states ->  Engine reads them.
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Ignore keys outside our array range
    if (key < 0 || key >= 1024) return;

    // Key was pressed this frame
    if (action == GLFW_PRESS)
    {
        // Only mark "pressed" if it was NOT already down (prevents repeats)
        if (!keysDown[key])
            keysPressed[key] = true;

        keysDown[key] = true;
    }
    // Key was released this frame
    else if (action == GLFW_RELEASE)
    {
        keysDown[key] = false;
        keysReleased[key] = true;
    }
}


// Initialize
// Registers the GLFW key callback and stores the window pointer.
// Must be called ONCE after creating the window.
// This function connects your engine’s input system to the GLFW window
// so it starts receiving keyboard events.
void Input::Initialize(GLFWwindow* window)
{
    // Remember which window we are using for input
    internalWindow = window;
    // GLFW knows whenever a key is pressed or released on this window,
    // and it calls our KeyCallback function.”
    glfwSetKeyCallback(window, KeyCallback);
}

// BeginFrame
// Called once per frame at the start of the engine loop.
// Clears pressed/released states so they only last ONE frame.
void Input::BeginFrame()
{
    for (int i = 0; i < 1024; i++)
    {
        keysPressed[i] = false;
        keysReleased[i] = false;
    }
}

// Query Functions
// These are what the engine and gameplay systems call.

// Returns true every frame the key is physically held down.
bool Input::GetKeyDown(int key)
{
    return keysDown[key];
}

// Returns true ONLY on the frame the key transitions from UP → DOWN.
bool Input::GetKeyPressed(int key)
{
    return keysPressed[key];
}

// Returns true ONLY on the frame the key transitions from DOWN → UP.
bool Input::GetKeyReleased(int key)
{
    return keysReleased[key];
}
