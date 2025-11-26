#include <iostream>
#include "../include/Engine.h"
#include <btBulletDynamicsCommon.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../include/Renderer.h"
#include "../include/Input.h"
#include "../include/Camera.h"
#include "../include/CameraController.h"
#include "../include/GameTime.h"

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

    // Initialize Input System
    Input::Initialize(window);

    // Initialize Time System
    Time::Initialize();


    // Set background color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // Create and initialize renderer
    Renderer renderer;
    renderer.initialize();

    //create camera
    Camera camera(
        glm::vec3(0.0f, 0.0f, 5.0f),  //Position
        glm::vec3(0.0f, 1.0f, 0.0f),  //World up
		45.0f,                         //FOV
        -90.0f,                        //Yaw
        15.0f                           //Pitch
    );

    CameraController cameraController(camera, 5.0f, 0.1f);
    cameraController.setMode(CameraController::Mode::ORBIT);  // Start in orbit mode
    cameraController.setOrbitalCenter(glm::vec3(0.0f));
    cameraController.setOrbitalRadius(20.0f);

    // --- Window tracking ---
    bool isFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
    bool isMinimized = glfwGetWindowAttrib(window, GLFW_ICONIFIED) == GLFW_TRUE;
    int winW = 0, winH = 0, fbW = 0, fbH = 0;
    glfwGetWindowSize(window, &winW, &winH);
    glfwGetFramebufferSize(window, &fbW, &fbH);

    // --- Fixed timestep setup ---
    const double fixedDt = 1.0 / 60.0; // Fixed timestep 60fps
    double accumulator = 0.0; // Collects the elapsed time to decide when to run the next physics update
    int physicsSteps = 0;
    double physicsTimer = 0.0;

    // Counters for testing
    double physicsTime = 0.0;

    std::cout << "Renderer initialized, entering main loop" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "F1 - Toggle camera mode (Orbit/Free)" << std::endl;
    std::cout << "WASD - Move (Free mode)" << std::endl;
    std::cout << "Space/Ctrl - Up/Down (Free mode)" << std::endl;
    std::cout << "Mouse - (Not yet implemented)" << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        //Update time (calculates deltaTime automatically) 
        Time::Update();
        float deltaTime = Time::GetDeltaTime();
        // Reset per-frame input states
        Input::BeginFrame();

        //poll for input events 
        glfwPollEvents();

        

        // Add the elapsed frame time into the accumulator
        accumulator += deltaTime;
        // --- Fixed timestep updates ---
        // ====================
        // Run physics updates in fixed 1/60s steps until caught up with real time
        while (accumulator >= fixedDt)
        {
            simulate(fixedDt); // advance simulation by one fixed step
            accumulator -= fixedDt; // remove one step’s worth of time from the bucket
            physicsSteps++; // count how many physics updates ran this second
        }

        // ========================
        // Print how many physics steps occurred every second
        physicsTime += deltaTime;
        if (physicsTime >= 1.0)
        {
            std::cout << "Physics steps per second: " << physicsSteps << std::endl;
            physicsTime = 0.0;
            physicsSteps = 0;
        }

        if (Input::GetKeyPressed(GLFW_KEY_ENTER))
        {
            if (cameraController.getMode() == CameraController::Mode::ORBIT) {
                cameraController.setMode(CameraController::Mode::FREE);
                std::cout << "Camera mode: FREE (WASD only - mouse not yet wired up)" << std::endl;
            }
            else {
                cameraController.setMode(CameraController::Mode::ORBIT);
                std::cout << "Camera mode: ORBIT" << std::endl;
            }
        }
		//Update camera controller
        cameraController.update(deltaTime);
        // --- Window
        glfwGetFramebufferSize(window, &fbW, &fbH);

        // --- Render ---
        renderer.draw(fbW, fbH,camera);
        glfwSwapBuffers(window);

		//limit FPS if enabled
        Time::WaitForNextFrame();
    }

    std::cout << "Exiting..." << std::endl;
    renderer.cleanup();
    glfwTerminate();
    return 0;
}
