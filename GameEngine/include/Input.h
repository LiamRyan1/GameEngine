#pragma once
#include <GLFW/glfw3.h>

class CameraController;

namespace Input
{
    // Must be called once after window creation.
    // Registers all GLFW callbacks for keyboard and mouse input
    void Initialize(GLFWwindow* window);

    // Called once per frame.
    // Clears per-frame input transitions (pressed / released)
    void BeginFrame();

    // Keyboard Input
    bool GetKeyDown(int key);         // is held
    bool GetKeyPressed(int key);      // pressed this frame
    bool GetKeyReleased(int key);     // released this frame

    // Mouse Input
    bool GetMouseDown(int button);        // held
    bool GetMousePressed(int button);     // pressed this frame
    bool GetMouseReleased(int button);    // released this frame

    // Sets the active camera controller to receive mouse movement.
    // Used for editor orbit / free-look camera control
    void SetCameraController(CameraController* controller);
}
