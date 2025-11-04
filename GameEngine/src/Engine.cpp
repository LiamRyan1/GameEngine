#include <iostream>
#include "../include/Engine.h"
#include <btBulletDynamicsCommon.h>
#include <GLFW/glfw3.h>

int Start(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(640, 480, "Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Window State Tracking
    // Read and store the initial state.
    bool isFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
    bool isMinimized = glfwGetWindowAttrib(window, GLFW_ICONIFIED) == GLFW_TRUE;

    int winW = 0, winH = 0;   // window size in screen coords (points)
    int fbW = 0, fbH = 0;   // framebuffer size in pixels (for renderer later)
    // Setting to 0 prevents using uninitialized memory. GLFW updates them to 640, 480.
    // Seeing 0×0 in your logs tells you window size wasn’t retrieved.

    // Get the current logical window size in screen coordinates (points)
    // This is how big the window appears on screen. & is a pointer
    glfwGetWindowSize(window, &winW, &winH);

    // The framebuffer is the block of memory where OpenGL draws the final image each frame.
    // It represents the actual pixels that get displayed on the window.
    glfwGetFramebufferSize(window, &fbW, &fbH);

    // https://www.glfw.org/docs/3.3/

    // Keep last-known copies for change detection.
    // GLFW doesn't store history, only holds current values.
    bool lastFocused = isFocused; // Remember if the window was focused
    bool lastMinimized = isMinimized; // Remember if the window was minimized
    int  lastWinW = winW, lastWinH = winH; // Remember last known window size (points)
    int  lastFbW = fbW, lastFbH = fbH; // Remember last known framebuffer size (pixels)

    while (!glfwWindowShouldClose(window))
    {
        // Check for window and input events
        glfwPollEvents();

        // Get current window info
        isFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE; // check if window is active
        isMinimized = glfwGetWindowAttrib(window, GLFW_ICONIFIED) == GLFW_TRUE; // check if window is minimized
        glfwGetWindowSize(window, &winW, &winH);    // window size on screen
        glfwGetFramebufferSize(window, &fbW, &fbH); // pixel size for rendering

        // Compare with last frame to see if anything changed
        if (isFocused != lastFocused) {
            // Is the window still the focus
            // Later could potentially pause when the window loses focus
            lastFocused = isFocused;
        }

        if (isMinimized != lastMinimized) {
            // Has the window been minimized
            // Later could potentially pause the window if it get minimized
            lastMinimized = isMinimized;
        }

        if (winW != lastWinW || winH != lastWinH) {
            // Has the window size been changed
            // If it has the renderer needs to be notified
            lastWinW = winW;
            lastWinH = winH;
        }

        if (fbW != lastFbW || fbH != lastFbH) {
            // Has the Framebuffer size changed
            lastFbW = fbW;
            lastFbH = fbH;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);  // Red vertex
        glVertex2f(-0.5f, -0.5f);

        glColor3f(0.0f, 1.0f, 0.0f);  // Green vertex
        glVertex2f(0.5f, -0.5f);

        glColor3f(0.0f, 0.0f, 1.0f);  // Blue vertex
        glVertex2f(0.0f, 0.5f);
        glEnd();

        glfwSwapBuffers(window);
        
    }

    glfwTerminate();
    return 0;
}

// https://gafferongames.com/post/fix_your_timestep/
// https://en.cppreference.com/w/cpp/header/chrono.html
