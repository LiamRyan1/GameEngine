#pragma once
#include <GLFW/glfw3.h>

class CameraController;

namespace Input
{
    void Initialize(GLFWwindow* window);
    void BeginFrame();

    bool GetKeyDown(int key);         // is held
    bool GetKeyPressed(int key);      // pressed this frame
    bool GetKeyReleased(int key);     // released this frame

    // Mouse support
    void SetCameraController(CameraController* controller);
}
