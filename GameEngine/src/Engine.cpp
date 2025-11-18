#include <iostream>
#include <chrono>
#include "../include/Engine.h"
#include <btBulletDynamicsCommon.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../include/Renderer.h"
#include "../include/Input.h"
#include "../include/Camera.h"

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

static bool keysDown[1024] = { false };       // is currently down
static bool keysPressed[1024] = { false };    // was pressed this frame
static bool keysReleased[1024] = { false };   // was released this frame

// This function updates the key states every time a key is pressed or released
// so the engine knows exactly what happened.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Cant press a key out of bounds
    if (key < 0 || key >= 1024) return;

    if (action == GLFW_PRESS)
    {
        if (!keysDown[key])
            keysPressed[key] = true; // just pressed
        keysDown[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        keysDown[key] = false;
        keysReleased[key] = true; // just released
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

    glfwSetKeyCallback(window, keyCallback);

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
        0.0f                           //Pitch
    );

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
        // Reset per-frame input states
        Input::BeginFrame();

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
		//distance from origin
        const float radius = 10.0f;
		//calculate camera position in circular path around origin 
		//use sin and cos to smoothly vary x and z coordinates over time
        float camX = sin(glfwGetTime()) * radius;
        float camZ = cos(glfwGetTime()) * radius;

		//set the new camera position
        camera.setPosition(glm::vec3(camX, 0.0f, camZ));

		//make the camera look at the origin
        glm::vec3 direction = glm::normalize(glm::vec3(0.0f) - camera.getPosition());

		//update yaw and pitch based on direction vector towards origin
        float yaw = glm::degrees(atan2(direction.z, direction.x));
        float pitch = glm::degrees(asin(direction.y));
		//set the new yaw and pitch and update camera vectors
        camera.setYaw(yaw);
        camera.setPitch(pitch);



        // --- Window and input events ---
        glfwPollEvents();
        glfwGetFramebufferSize(window, &fbW, &fbH);

        // --- TEST INPUT ---
        if (Input::GetKeyPressed(GLFW_KEY_SPACE))
            std::cout << "SPACE pressed\n";

        if (Input::GetKeyDown(GLFW_KEY_SPACE))
            std::cout << "SPACE held\n";

        if (Input::GetKeyReleased(GLFW_KEY_SPACE))
            std::cout << "SPACE released\n";

        // --- Render ---
        renderer.draw(fbW, fbH,camera);
        glfwSwapBuffers(window);
    }

    std::cout << "Exiting..." << std::endl;
    renderer.cleanup();
    glfwTerminate();
    return 0;
}
