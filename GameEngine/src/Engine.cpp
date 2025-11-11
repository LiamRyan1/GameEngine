#include <iostream>
#include <chrono>
#include "../include/Engine.h"
#include <btBulletDynamicsCommon.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../include/Renderer.h"


void simulate(double dt)
{
    static double totalTime = 0.0;
    totalTime += dt;

    // Print message every simulated second
    if (static_cast<int>(totalTime) % 1 == 0)
    {
        // Placeholder for future physics updates
        // std::cout << "Simulating at t = " << totalTime << "s" << std::endl;
    }
}

int Start(void)
{
    GLFWwindow* window;

    if (!glfwInit())
    {
        std::cout << "Failed to init GLFW" << std::endl;
        return -1;
    }

    std::cout << "GLFW initialized" << std::endl;

    // Request OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(640, 480, "Game Engine", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "Window created" << std::endl;
    glfwMakeContextCurrent(window);

    std::cout << "Initializing GLEW..." << std::endl;
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cout << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "GLEW initialized successfully" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Set background color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // Create and initialize renderer
    Renderer renderer;
    renderer.initialize();

    // --- Window tracking ---
    bool isFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
    bool isMinimized = glfwGetWindowAttrib(window, GLFW_ICONIFIED) == GLFW_TRUE;
    int winW = 0, winH = 0, fbW = 0, fbH = 0;
    glfwGetWindowSize(window, &winW, &winH);
    glfwGetFramebufferSize(window, &fbW, &fbH);

    // --- Fixed timestep setup ---
    using clock = std::chrono::high_resolution_clock; // Timer
    auto currentTime = clock::now(); // Stores the start time of the loop
    double t = 0.0; // Total time that has passed
    const double dt = 1.0 / 60.0; // Fixed timestep 60fps
    double accumulator = 0.0; // Collects the elapsed time to decide when to run the next physics update

    // Counters for testing
    double physicsTime = 0.0;
    int physicsSteps = 0;

    std::cout << "Renderer initialized, entering main loop" << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        // --- Frame timing ---
        // =======================
        // Measure how long the last frame took
        auto newTime = clock::now();
        // Calculate how much real time passed since the last frame
        std::chrono::duration<double> frameDuration = newTime - currentTime;
        // Convert that duration into seconds (as a double value)
        double frameTime = frameDuration.count();
        // Update the stored "current time" for the next frame
        currentTime = newTime;

        // Limit max frame time to 0.25s
        if (frameTime > 0.25)
            frameTime = 0.25; // Prevent spiral of death

        // Add the elapsed frame time into the accumulator
        accumulator += frameTime;

        // --- Fixed timestep updates ---
        // ====================
        // Run physics updates in fixed 1/60s steps until caught up with real time
        while (accumulator >= dt)
        {
            simulate(dt); // advance simulation by one fixed step
            accumulator -= dt; // remove one step’s worth of time from the bucket
            t += dt; // track total simulated time
            physicsSteps++; // count how many physics updates ran this second
        }

        // ========================
        // Print how many physics steps occurred every second
        physicsTime += frameTime;
        if (physicsTime >= 1.0)
        {
            std::cout << "Physics steps per second: " << physicsSteps << std::endl;
            physicsTime = 0.0;
            physicsSteps = 0;
        }

        // --- Window and input events ---
        glfwPollEvents();
        glfwGetFramebufferSize(window, &fbW, &fbH);

        // --- Render ---
        renderer.draw(fbW, fbH);
        glfwSwapBuffers(window);
    }

    std::cout << "Exiting..." << std::endl;
    renderer.cleanup();
    glfwTerminate();
    return 0;
}
