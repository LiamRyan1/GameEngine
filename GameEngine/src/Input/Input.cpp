#include "../include/Input/Input.h"
#include "../include/Input/CameraController.h"
#include "../external/imgui/backends/imgui_impl_glfw.h"
#include "../external/imgui/core/imgui.h"


// keysDown[key]     → true while key is being held
// keysPressed[key]  → true for ONE frame when key transitions RELEASED → PRESSED
// keysReleased[key] → true for ONE frame when key transitions PRESSED → RELEASED
// They are reset every frame for pressed/released transitions.
// 1024 stores key states
static bool keysDown[1024] = { false }; //is currently down
static bool keysPressed[1024] = { false };//was pressed this frame
static bool keysReleased[1024] = { false }; //was released this frame

static bool mouseButtonsDown[8] = { false }; //is currently down
static bool mouseButtonsPressed[8] = { false }; //was pressed this frame
static bool mouseButtonsReleased[8] = { false }; //was released this frame

// GLFW window pointer, stored so the input module knows which window it belongs to.
// It’s the Input system’s saved reference to the game window.
// Without it, we can’t read input from GLFW.
static GLFWwindow* internalWindow = nullptr;

// Pointer to active CameraController
static CameraController* s_CameraController = nullptr;

static double lastMouseX = 0.0;
static double lastMouseY = 0.0;
static double mouseDeltaX = 0.0;
static double mouseDeltaY = 0.0;
static bool firstMouse = true;

// KeyCallback
// This function updates the key states every time a key is pressed or released
// so the engine knows exactly what happened.
// This is called directly by GLFW every time a key's state changes.
// We only update states here; the engine will read them later.
// GLFW calls KeyCallback -> Update the key states ->  Engine reads them.
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{   
    // Forward event to ImGui
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    // Cant press a key out of bounds
    // Ignore keys outside our array range
    if (key < 0 || key >= 1024) return;

    // Key was pressed this frame
    if (action == GLFW_PRESS)
    {
        // Only mark "pressed" if it was NOT already down (prevents repeats)
        if (!keysDown[key])
            keysPressed[key] = true;// just pressed
         keysDown[key] = true;
    }
    // Key was released this frame
    else if (action == GLFW_RELEASE)
    {
        keysDown[key] = false;
        keysReleased[key] = true;// just released
    }
}

// Mouse movement callback
static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if (firstMouse)
    {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    mouseDeltaX = xpos - lastMouseX;
    mouseDeltaY = ypos - lastMouseY;

    lastMouseX = xpos;
    lastMouseY = ypos;

    // Only forward to CameraController in Editor mode
    if (s_CameraController)
    {
        if (!ImGui::GetIO().WantCaptureMouse &&
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            s_CameraController->processMouse(xpos, ypos);
        }
        else
        {
            s_CameraController->resetMouseTracking();
        }
    }
}

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    if (button < 0 || button >= 8) return;

    if (action == GLFW_PRESS)
    {
        if (!mouseButtonsDown[button])
            mouseButtonsPressed[button] = true;

        mouseButtonsDown[button] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        mouseButtonsDown[button] = false;
        mouseButtonsReleased[button] = true;
    }
}

static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

static void CharCallback(GLFWwindow* window, unsigned int c)
{
    ImGui_ImplGlfw_CharCallback(window, c);
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
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetCharCallback(window, CharCallback);
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

    for (int i = 0; i < 8; i++)
    {
        mouseButtonsPressed[i] = false;
        mouseButtonsReleased[i] = false;
    }

    mouseDeltaX = 0.0;
    mouseDeltaY = 0.0;

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

// Allow engine to set active camera controller
void Input::SetCameraController(CameraController* controller)
{
    s_CameraController = controller;
}

bool Input::GetMouseDown(int button)
{
    if (button < 0 || button >= 8) return false;
    return mouseButtonsDown[button];
}

bool Input::GetMousePressed(int button)
{
    if (button < 0 || button >= 8) return false;
    return mouseButtonsPressed[button];
}

bool Input::GetMouseReleased(int button)
{
    if (button < 0 || button >= 8) return false;
    return mouseButtonsReleased[button];
}

double Input::GetMouseDeltaX()
{
    return mouseDeltaX;
}

double Input::GetMouseDeltaY()
{
    return mouseDeltaY;
}

