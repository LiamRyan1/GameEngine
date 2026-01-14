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
#include "../include/Physics.h"
#include "../include/Scene.h"
#include "../include/Debug/DebugUI.h"
#include "../include/Debug/DebugUIContext.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../include/Raycast.h"



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

    // Tracks whether the engine is currently running in editor mode
    // Editor mode (UI + selection) or game mode (gameplay + camera control).
    EngineMode engineMode = EngineMode::Editor;

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

    window = glfwCreateWindow(960, 720, "Game Engine", NULL, NULL);
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

    // ImGui initialization (Engine-owned)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize Input System
    Input::Initialize(window);

    // Initialize Time System
    Time::Initialize();
    Time::SetTargetFPS(60.0f);
    Time::EnableFPSLimit(true);

    // Set background color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // Create and initialize renderer
    Renderer renderer;
    renderer.initialize();

	// Create and initialize physics system
    Physics physics;
    physics.initialize();
    std::cout << "Physics world has " << physics.getRigidBodyCount()<< " rigid bodies" << std::endl;

    // Create scene manager
    Scene scene(physics);

    // x Remove this hardcoded scene setup when loading from files
    // Create ground plane as a visible GameObject
    scene.createCube(glm::vec3(0, -0.25f, 0), glm::vec3(100.0f, 0.5f, 100.0f), 0.0f);
    // Create some test cubes - only using cubes for now
    auto cube1 = scene.createCube(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f), 1.0f);
    cube1->setTexturePath("textures/stone-1024.jpg");
    auto cube2 = scene.createCube(glm::vec3(4.0f, 8.0f, -6.0f), glm::vec3(1.0f), 1.0f);
    cube2->setTexturePath("textures/wood1.jpg");
    auto cube3 = scene.createCube(glm::vec3(-3.0f, 6.0f, -5.0f), glm::vec3(1.0f), 1.0f);
    // no texture default to orange
    auto cube4 = scene.createCube(glm::vec3(-6.0f, 10.0f, -10.0f), glm::vec3(1.0f), 1.0f);
	cube4->setTexturePath("textures/texture_06.png");
    auto cube5 = scene.createCube(glm::vec3(5.0f, 12.0f, -7.0f), glm::vec3(1.0f), 1.0f);

	// currently only cubes are supported so spheres will be cubes too`but with sphere colliders
	auto sphere1 = scene.createSphere(glm::vec3(2.0f, 15.0f, 3.0f), 1.0f, 1.0f);

    // TODO: Replace hardcoded scene with file loading
    // scene.loadFromFile("scenes/test_level.json");


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
    cameraController.setOrbitalRadius(25.0f);

    // Tell Input which camera to rotate
    Input::SetCameraController(&cameraController);

    // Create Debug UI
    DebugUI debugUI;

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

    // ===================================
    // Main Loop
    // ===================================
    while (!glfwWindowShouldClose(window))
    {

        // UI Toggle flag
        static bool showUI = true;

        //Update time (calculates deltaTime automatically) 
        Time::Update();
        float deltaTime = Time::GetDeltaTime();
        
        // Reset per-frame input states
        Input::BeginFrame();

        //poll for input events 
        glfwPollEvents();

        // Toggle between Editor and Game modes.
        // Editor mode:
        //  - Mouse cursor is visible
        //  - ImGui and editor tools are active
        //  - Object selection is allowed
        // Game mode:
        //  - Mouse cursor is captured
        //  - Camera controls are active
        //  - Editor interaction is disabled
        if (Input::GetKeyPressed(GLFW_KEY_E))
        {
            if (engineMode == EngineMode::Editor)
            {
                engineMode = EngineMode::Game;

                // Capture the mouse for gameplay / camera control
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                std::cout << "Mode: GAME" << std::endl;
            }
            else
            {
                engineMode = EngineMode::Editor;

                // Release the mouse so the user can interact with the editor UI
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                std::cout << "Mode: EDITOR" << std::endl;
            }
        }


        // Add the elapsed frame time into the accumulator
        accumulator += deltaTime;
        // --- Fixed timestep updates ---
        // ====================
        // Run physics updates in fixed 1/60s steps until caught up with real time
        while (accumulator >= fixedDt)
        {
            physics.update(fixedDt); // advance simulation by one fixed step
            accumulator -= fixedDt; // remove one step’s worth of time from the bucket
            physicsSteps++; // count how many physics updates ran this second
        }
        scene.update();
        // ========================
        // Print how many physics steps occurred every second
        physicsTime += deltaTime;

        if (physicsTime >= 1.0)
        {
            std::cout << "Physics steps per second: " << physicsSteps << std::endl;
            physicsTime = 0.0;
            physicsSteps = 0;
        }
		// Camera mode toggle
        if (Input::GetKeyPressed(GLFW_KEY_F))
        {
            if (cameraController.getMode() == CameraController::Mode::ORBIT) {
                cameraController.setMode(CameraController::Mode::FREE);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                std::cout << "Camera mode: FREE" << std::endl;
            }
            else {
                cameraController.setMode(CameraController::Mode::ORBIT);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                std::cout << "Camera mode: ORBIT" << std::endl;
            }
        }

        // Allow ESC to unlock cursor
        if (Input::GetKeyPressed(GLFW_KEY_ESCAPE)) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

		//Update camera controller
        cameraController.update(deltaTime);
        // --- Window
        glfwGetFramebufferSize(window, &fbW, &fbH);

        // ===============================
        // Editor Ray → AABB Picking
        // ===============================
        if (engineMode == EngineMode::Editor &&
            Input::GetMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            // Mouse position
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // Reverse the rendering pipeline to build a ray from mouse click:
            // 1. Screen pixels (where user clicked)
            // 2. NDC (standard -1 to 1 range)
            // 3. Unproject using inverse camera matrices
            // 4. 3D ray in world space for intersection testing
            float x = (2.0f * static_cast<float>(mouseX)) / fbW - 1.0f;
            float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / fbH;

            // Create a point in clip space at the near clipping plane
            // (x, y) = NDC coordinates of mouse click
            // z = -1.0 = near plane (start of visible depth)
            // w = 1.0 = this is a POINT/position (not a direction yet)
            glm::vec4 rayClip(x, y, -1.0f, 1.0f);

            // Get the camera's projection matrix
            // Aspect ratio is needed to match screen dimensions
            glm::mat4 projection =
                camera.getProjectionMatrix(static_cast<float>(fbW) / fbH);

            // Unproject clip space to eye space, then convert to forward-pointing direction vector
            glm::vec4 rayEye = glm::inverse(projection) * rayClip;
            rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

            // Inverse view matrix transforms from eye space -> world space, then normalize to unit direction
            glm::vec3 rayDirection =
                glm::normalize(glm::vec3(glm::inverse(camera.getViewMatrix()) * rayEye));

            // Ray starts at the camera's position in world space
            glm::vec3 rayOrigin = camera.getPosition();

            std::cout << "[Editor Ray]\n";
            std::cout << "  Mouse: (" << mouseX << ", " << mouseY << ")\n";
            std::cout << "  Origin: (" << rayOrigin.x << ", "
                << rayOrigin.y << ", "
                << rayOrigin.z << ")\n";
            std::cout << "  Direction: (" << rayDirection.x << ", "
                << rayDirection.y << ", "
                << rayDirection.z << ")\n";

            // Shoot the laser through the scene. Check every object. Did laser hit any? Keep closest one. What object did laser hit, if any.
            // Test ray against all scene objects to find closest hit
            float closestHit = FLT_MAX;

            // Loop through every object in the scene
            for (const auto& obj : scene.getObjects())
            {
                // Get object's world position and size
                const glm::vec3& position = obj->getPosition();
                const glm::vec3& scale = obj->getScale();

                // Build AABB (bounding box) from object's transform
                // Assumes object origin is at center
                glm::vec3 aabbMin = position - scale * 0.5f;
                glm::vec3 aabbMax = position + scale * 0.5f;

                float hitDistance;
                // Test if ray intersects this object's bounding box
                if (RayIntersectsAABB(
                    rayOrigin,
                    rayDirection,
                    aabbMin,
                    aabbMax,
                    hitDistance))
                {
                    // Keep track of the closest object hit so far
                    if (hitDistance < closestHit)
                    {
                        closestHit = hitDistance;
                    }
                }
            }
            // Report results if we hit something
            if (closestHit < FLT_MAX)
            {
                std::cout << "[Editor Pick] HIT at distance "
                    << closestHit << "\n";
            }
        }

        // Build Debug UI Context
        // This packages up read-only engine data and approved debug commands
        // so DebugUI can display stats and issue requests without owning systems.
        DebugUIContext uiContext;

        // Timing / performance data
        uiContext.time.deltaTime = Time::GetDeltaTime();
        uiContext.time.fps = Time::GetFPS();

        // Physics debug data
        uiContext.physics.rigidBodyCount = physics.getRigidBodyCount();
        uiContext.physics.physicsEnabled = true;

        // Scene debug commands
        // Wire DebugUI requests to real scene actions.
        // This keeps DebugUI decoupled from Scene and Physics ownership.
        uiContext.scene.spawnCube =
            [&scene](const glm::vec3& position, bool withPhysics)
            {
                scene.createCube(
                    position,
                    glm::vec3(1.0f),
                    withPhysics ? 1.0f : 0.0f
                );
            };

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Draw Debug UI (logic only)
        debugUI.draw(uiContext);

        // End ImGui frame
        ImGui::Render();

        // --- Render ---
        renderer.draw(fbW, fbH,camera, scene.getObjects());
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

		//limit FPS if enabled
        Time::WaitForNextFrame();
    }

    std::cout << "Exiting..." << std::endl;

    // ImGui shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    renderer.cleanup();
    physics.cleanup();
    glfwTerminate();
    return 0;
}
