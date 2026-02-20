#include "../include/Misc/FileUtils.h"
#include <iostream>
#include "../include/Core/Engine.h"
#include <btBulletDynamicsCommon.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "../include/Rendering/Renderer.h"
#include "../include/Input/Input.h"
#include "../include/Rendering/Camera.h"
#include "../include/Input/CameraController.h"
#include "../include/Core/GameTime.h"
#include "../include/Physics/Physics.h"
#include "../include/Scene/Scene.h"
#include "../include/Debug/DebugUI.h"
#include "../include/Debug/DebugUIContext.h"
#include "../External/imgui/core/imgui.h"
#include "../External/imgui/backends/imgui_impl_glfw.h"
#include "../External/imgui/backends/imgui_impl_opengl3.h"
#include "../include/UI/Raycast.h"
#include "../include/Physics/PhysicsMaterial.h"
#include "../include/Editor/Gizmo.h"
#include "../include/Physics/ConstraintRegistry.h"
#include "../include/Physics/ConstraintPreset.h"
#include "../include/Physics/ConstraintTemplate.h"
#include "../include/Saves/SceneSavePanel.h"
#include <filesystem>


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

    // Currently selected object in editor mode.
    // This is editor-only state and does NOT belong in Renderer or Scene.
    std::vector<GameObject*> selectedObjects;

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
    glfwSwapInterval(0);

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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize Input System
    Input::Initialize(window);

    // Initialize Time System
    Time::Initialize();
    Time::SetTargetFPS(60.0f);
  

    // Set background color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // Create and initialize renderer
    Renderer renderer;
    renderer.initialize();

    // Load skybox
    std::vector<std::string> skyboxFaces = {
        "textures/skybox/right.jpg",
        "textures/skybox/left.jpg",
        "textures/skybox/top.jpg",
        "textures/skybox/bottom.jpg",
        "textures/skybox/front.jpg",
        "textures/skybox/back.jpg"
    };

    if (renderer.loadSkybox(skyboxFaces)) {
        std::cout << "Skybox loaded successfully!" << std::endl;
    }
    else {
        std::cout << "Failed to load skybox" << std::endl;
    }

	// Create and initialize physics system
    Physics physics;
    physics.initialize();
    ConstraintRegistry::getInstance().initialize(physics.getWorld());
    std::cout << "Physics world has " << physics.getRigidBodyCount()<< " rigid bodies" << std::endl;
    // Initialize constraint templates
    ConstraintTemplateRegistry::getInstance().load();
    ConstraintTemplateRegistry::getInstance().initializeDefaults();
    std::cout << "Loaded " << ConstraintTemplateRegistry::getInstance().getTemplateCount()
        << " constraint templates" << std::endl;

    // Create scene manager
    Scene scene(physics, renderer);

    // x Remove this hardcoded scene setup when loading from files
    // Create ground plane as a visible GameObject
    scene.spawnObject(ShapeType::CUBE, glm::vec3(0, -0.25f, 0), glm::vec3(100.0f, 0.5f, 100.0f), 0.0f, "Default");
    // Create some test cubes
    auto cube1 = scene.spawnObject(ShapeType::CUBE, glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f), 1.0f, "Metal", "textures/stone-1024.jpg");
    auto cube2 = scene.spawnObject(ShapeType::CUBE, glm::vec3(4.0f, 8.0f, -6.0f), glm::vec3(1.0f), 1.0f, "Wood", "textures/wood1.jpg");
    auto cube3 = scene.spawnObject(ShapeType::CUBE, glm::vec3(-3.0f, 6.0f, -5.0f), glm::vec3(1.0f), 1.0f, "Rubber");
    // no texture - default to orange
    auto cube4 = scene.spawnObject(ShapeType::CUBE, glm::vec3(-6.0f, 10.0f, -10.0f), glm::vec3(1.0f), 1.0f, "Ice", "textures/texture_06.png");
    auto cube5 = scene.spawnObject(ShapeType::CUBE, glm::vec3(5.0f, 12.0f, -7.0f), glm::vec3(1.0f), 1.0f, "Plastic");
    // Create a sphere (currently rendered as cube with sphere collider)
    auto sphere1 = scene.spawnObject(ShapeType::SPHERE, glm::vec3(2.0f, 15.0f, 3.0f), glm::vec3(1.0f), 1.0f, "Rubber");

    scene.spawnRenderObject(
        ShapeType::CUBE,
        glm::vec3(0, 3, 0),
        glm::vec3(1.0f),
        "textures/wood1.jpg"
    );


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

    // Create gizmo
    EditorGizmo gizmo;

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

                // --- WAKE ALL PHYSICS BODIES ---
                for (auto& obj : scene.getObjects())
                {
                    if (obj->hasPhysics())
                        obj->getRigidBody()->activate(true);
                }
            }
            else
            {
                engineMode = EngineMode::Editor;

                // Release the mouse so the user can interact with the editor UI
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                std::cout << "Mode: EDITOR" << std::endl;
            }

        }

        if (Input::GetKeyPressed(GLFW_KEY_V))  // 'V' for Visualize
        {
            renderer.toggleDebugPhysics();
            std::cout << "Debug Physics Wireframes: " << (renderer.isDebugPhysicsEnabled() ? "ON" : "OFF") << std::endl;
        }

        if (Input::GetKeyPressed(GLFW_KEY_F5))
        {
            scene.saveToFile("../../assets/scenes/scene_test.json");
        }

        if (Input::GetKeyPressed(GLFW_KEY_F9))
        {
            scene.loadFromFile("../../assets/scenes/scene_test.json");
        }


        // Add the elapsed frame time into the accumulator
        accumulator += deltaTime;
        // --- Fixed timestep updates ---
        // ====================
        // Run physics updates in fixed 1/60s steps until caught up with real time
        if (engineMode == EngineMode::Game)
        {
            while (accumulator >= fixedDt)
            {
                physics.update(fixedDt); // advance simulation by one fixed step
                accumulator -= fixedDt; // remove one step’s worth of time from the bucket
                physicsSteps++; // count how many physics updates ran this second
            }
        }
        else
        {
            // In editor mode, discard accumulator so physics doesn't "catch up"
            accumulator = 0.0;
        }
        scene.update(engineMode);

        if (engineMode == EngineMode::Editor &&
            !selectedObjects.empty() &&
            Input::GetKeyPressed(GLFW_KEY_DELETE) &&
            !ImGui::GetIO().WantCaptureKeyboard)
        {
            for (GameObject* obj : selectedObjects)
                scene.requestDestroy(obj);

            selectedObjects.clear();
        }


        if (Input::GetKeyPressed(GLFW_KEY_G)) {  // Press G to test grid
            std::cout << "\n=== SPATIAL GRID TEST ===" << std::endl;

            // 1. Show grid stats
            scene.printSpatialStats();

            // 2. Test query around camera
            glm::vec3 testPos = camera.getPosition();
            auto nearby = scene.findObjectsInRadius(testPos, 20.0f);

            std::cout << "\nObjects within 20 units of camera:" << std::endl;
            std::cout << "Found: " << nearby.size() << " objects" << std::endl;

            for (GameObject* obj : nearby) {
                glm::vec3 pos = obj->getPosition();
                float dist = glm::distance(pos, testPos);
                std::cout << "  - Distance: " << dist << " units" << std::endl;
            }

            std::cout << "===================\n" << std::endl;
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

		
        // --- Window
        glfwGetFramebufferSize(window, &fbW, &fbH);

        // Start ImGui frame BEFORE using io.WantCaptureMouse.
        // Otherwise uiWantsMouse is either invalid or from last frame.
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Working Directory");
        ImGui::Text("%s", std::filesystem::current_path().string().c_str());
        ImGui::End();


        // ImGui tells us if the UI wants mouse input this frame
        // (hovering, dragging sliders, clicking windows, etc.)
       // Check if mouse is over any ACTUAL panel (not the invisible dockspace)
        bool uiWantsMouse =
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) ||
            ImGui::IsAnyItemHovered() ||
            ImGui::IsAnyItemActive();

        GameObject* primarySelection =
            selectedObjects.empty() ? nullptr : selectedObjects.front();

        //Update camera controller
        cameraController.update(deltaTime);

        bool gizmoCapturingMouse = gizmo.update(
            window, fbW, fbH,
            camera,
            primarySelection,
            engineMode == EngineMode::Editor,
            uiWantsMouse
        );


        // Gizmo visuals
        if (engineMode == EngineMode::Editor && primarySelection)
        {
            gizmo.draw(fbW, fbH, camera, primarySelection);
        }

        // In your main loop, around where you handle input:
        if (engineMode == EngineMode::Game) {
            // Test raycast when you press R key
            if (Input::GetKeyPressed(GLFW_KEY_R)) {
                PhysicsQuery& query = physics.getQuerySystem();

                // Raycast from camera forward
                glm::vec3 from = camera.getPosition();
                glm::vec3 to = from + camera.getFront() * 100.0f;

                RaycastHit hit;
                if (query.raycast(from, to, hit)) {
                    std::cout << "HIT: " << hit.object->getName()
                        << " at distance " << hit.distance << "m" << std::endl;
                }
                else {
                    std::cout << "MISS" << std::endl;
                }
            }
        }
        // ===============================
        // Editor Ray -> AABB Picking
        // ===============================
        if (engineMode == EngineMode::Editor && 
            !uiWantsMouse &&
            !gizmoCapturingMouse &&
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
            GameObject* hitObject = nullptr;

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
                        hitObject = obj.get();
                    }
                }
            }

            // If the ray hit something, update the editor selection
            if (hitObject)
            {
                bool shiftHeld =
                    Input::GetKeyDown(GLFW_KEY_LEFT_SHIFT) ||
                    Input::GetKeyDown(GLFW_KEY_RIGHT_SHIFT);

                if (!shiftHeld)
                    selectedObjects.clear();

                // Avoid duplicates
                if (std::find(selectedObjects.begin(),
                    selectedObjects.end(),
                    hitObject) == selectedObjects.end())
                {
                    selectedObjects.push_back(hitObject);
                }
            }
            else
            {
                // Click empty space clears selection
                selectedObjects.clear();
            }


        }

        // Build Debug UI Context
        // This packages up read-only engine data and approved debug commands
        // so DebugUI can display stats and issue requests without owning systems.
        DebugUIContext uiContext;

        // Pass editor selection into the UI so Inspector can display it.
        // DebugUI does not decide selection — it only reacts to it.
        uiContext.selectedObject = primarySelection;

        // Timing / performance data
        uiContext.time.deltaTime = Time::GetDeltaTime();
        uiContext.time.fps = Time::GetFPS();

        // Physics debug data
        uiContext.physics.rigidBodyCount = physics.getRigidBodyCount();
        uiContext.physics.physicsEnabled = true;

        // Populate available materials from the registry
        uiContext.physics.availableMaterials = MaterialRegistry::getInstance().getAllMaterialNames();
        
		// generic spawn function
        uiContext.scene.spawnObject =
            [&scene](ShapeType type, const glm::vec3& position,
                const glm::vec3& size, float mass,
                const std::string& materialName,
                const std::string& texturePath)
            {
                auto* obj = scene.spawnObject(type, position, size, mass, materialName);
                if (!texturePath.empty()) {
                    obj->setTexturePath(texturePath);
                }
            };

        // Spawn for objects without physics
        uiContext.scene.spawnRenderObject =
            [&scene](ShapeType type, const glm::vec3& pos, const glm::vec3& size, const std::string& tex)
            {
                return scene.spawnRenderObject(type, pos, size, tex);
            };

        // Register custom material command
        uiContext.scene.registerMaterial =
            [](const std::string& name, float friction, float restitution)
            {
                PhysicsMaterial customMat(name, friction, restitution);
                MaterialRegistry::getInstance().registerMaterial(customMat);
            };
		// Set object scale command (handles both render and physics resizing)
        uiContext.scene.setObjectScale =
            [&scene](GameObject* obj, const glm::vec3& scale) {
            scene.setObjectScale(obj, scale);
            };

        // Destroy object (deferred, editor-safe)
        uiContext.scene.destroyObject =
            [&scene, &selectedObjects](GameObject* obj)
            {
                if (!obj) return;

                // Remove from selection list if present
                selectedObjects.erase(
                    std::remove(selectedObjects.begin(), selectedObjects.end(), obj),
                    selectedObjects.end()
                );

                scene.requestDestroy(obj);
            };


        // Get available textures command
        uiContext.scene.getAvailableTextures =
            []() -> std::vector<std::string>
            {
                return FileUtils::getTextureFiles("textures");
            };

		// Lighting commands
        uiContext.lighting.getLight = [&renderer]() -> DirectionalLight& {
            return renderer.getLight();
            };

        // Get available models command
        uiContext.scene.getAvailableModels =
            []() -> std::vector<std::string> {
            return FileUtils::getModelFiles("models");
            };

        // Model loading command
        uiContext.scene.loadAndSpawnModel =
            [&scene](const std::string& filepath, const glm::vec3& pos, const glm::vec3& meshScale,
                bool enablePhysics,
                float mass,
                const glm::vec3& physicsBoxScale,
                const std::string& materialName) -> GameObject* {
            return scene.loadAndSpawnModel(filepath, pos, meshScale, enablePhysics, mass, physicsBoxScale, materialName);
            };

        uiContext.scene.setObjectPhysicsScale = [&scene](GameObject* obj, const glm::vec3& scale) {
            scene.setObjectPhysicsScale(obj, scale);
            };
        // ===== Constraint System Commands =====
        auto& registry = ConstraintRegistry::getInstance();

        // Update constraint view state
        uiContext.constraints.totalConstraints = registry.getConstraintCount();
        uiContext.constraints.allConstraints = registry.getAllConstraints();

        // Count active/broken
        uiContext.constraints.activeConstraints = 0;
        uiContext.constraints.brokenConstraints = 0;
        for (Constraint* c : uiContext.constraints.allConstraints) {
            if (c->isBroken()) {
                uiContext.constraints.brokenConstraints++;
            }
            else {
                uiContext.constraints.activeConstraints++;
            }
        }

        // Count by type
        uiContext.constraints.fixedCount = registry.findConstraintsByType(ConstraintType::FIXED).size();
        uiContext.constraints.hingeCount = registry.findConstraintsByType(ConstraintType::HINGE).size();
        uiContext.constraints.sliderCount = registry.findConstraintsByType(ConstraintType::SLIDER).size();
        uiContext.constraints.springCount = registry.findConstraintsByType(ConstraintType::SPRING).size();
        uiContext.constraints.dof6Count = registry.findConstraintsByType(ConstraintType::GENERIC_6DOF).size();

        // === Creation Commands ===

        uiContext.constraintCommands.createFixed =
            [&registry](GameObject* objA, GameObject* objB) -> Constraint* {
            auto constraint = ConstraintPreset::createFixed(objA, objB);
            return constraint ? registry.addConstraint(std::move(constraint)) : nullptr;
            };

        uiContext.constraintCommands.createHinge =
            [&registry](GameObject* objA, GameObject* objB,
                const glm::vec3& worldPivot, const glm::vec3& worldAxis) -> Constraint* {
                    auto constraint = ConstraintPreset::createHinge(objA, objB, worldPivot, worldAxis);
                    return constraint ? registry.addConstraint(std::move(constraint)) : nullptr;
            };

        uiContext.constraintCommands.createHingeAdvanced =
            [&registry](GameObject* objA, GameObject* objB, const HingeParams& params) -> Constraint* {
            auto constraint = ConstraintPreset::createHinge(objA, objB, params);
            return constraint ? registry.addConstraint(std::move(constraint)) : nullptr;
            };

        uiContext.constraintCommands.createSlider =
            [&registry](GameObject* objA, GameObject* objB, const SliderParams& params) -> Constraint* {
            auto constraint = ConstraintPreset::createSlider(objA, objB, params);
            return constraint ? registry.addConstraint(std::move(constraint)) : nullptr;
            };

        uiContext.constraintCommands.createSpring =
            [&registry](GameObject* objA, GameObject* objB, float stiffness, float damping) -> Constraint* {
            auto constraint = ConstraintPreset::createSpring(objA, objB, stiffness, damping);
            return constraint ? registry.addConstraint(std::move(constraint)) : nullptr;
            };

        uiContext.constraintCommands.createSpringAdvanced =
            [&registry](GameObject* objA, GameObject* objB, const SpringParams& params) -> Constraint* {
            auto constraint = ConstraintPreset::createSpring(objA, objB, params);
            return constraint ? registry.addConstraint(std::move(constraint)) : nullptr;
            };

        uiContext.constraintCommands.createGeneric6Dof =
            [&registry](GameObject* objA, GameObject* objB, const Generic6DofParams& params) -> Constraint* {
            auto constraint = ConstraintPreset::createGeneric6Dof(objA, objB, params);
            return constraint ? registry.addConstraint(std::move(constraint)) : nullptr;
            };

        // === Management Commands ===

        uiContext.constraintCommands.removeConstraint =
            [&registry](Constraint* constraint) {
            registry.removeConstraint(constraint);
            };

        uiContext.constraintCommands.removeConstraintByName =
            [&registry](const std::string& name) -> bool {
            return registry.removeConstraint(name);
            };

        uiContext.constraintCommands.removeConstraintsForObject =
            [&registry](GameObject* obj) {
            registry.removeConstraintsForObject(obj);
            };

        uiContext.constraintCommands.clearAllConstraints =
            [&registry]() {
            registry.clearAll();
            };

        // === Query Commands ===

        uiContext.constraintCommands.findConstraintByName =
            [&registry](const std::string& name) -> Constraint* {
            return registry.findConstraintByName(name);
            };

        uiContext.constraintCommands.findConstraintsForObject =
            [&registry](GameObject* obj) -> std::vector<Constraint*> {
            return registry.findConstraintsByObject(obj);
            };

        uiContext.constraintCommands.findConstraintsByType =
            [&registry](ConstraintType type) -> std::vector<Constraint*> {
            return registry.findConstraintsByType(type);
            };

        // Draw Debug UI (logic only)
        debugUI.draw(uiContext);

        // Draw SceneSavePanel
        DrawSceneSaveLoadPanel(scene);

        // End ImGui frame
        ImGui::Render();

        // --- Render ---
        renderer.draw(fbW, fbH, camera, scene.getObjects(), primarySelection, selectedObjects);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

		//limit FPS if enabled
        Time::WaitForNextFrame();
    }

    std::cout << "Exiting..." << std::endl;

    // ImGui shutdown
    ConstraintTemplateRegistry::getInstance().save();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    renderer.cleanup();
    physics.cleanup();
    glfwTerminate();
    return 0;
}
