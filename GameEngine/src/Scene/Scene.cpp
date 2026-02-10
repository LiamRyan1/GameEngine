#include <GL/glew.h> 
#include "../include/Scene/Scene.h"
#include "../include/Core/Engine.h"
#include "../include/Physics/ConstraintRegistry.h"
#include <unordered_map> 
#include <iostream>


/**
 * @brief Constructs a new Scene with a reference to the physics world.
 *
 * The Scene does not own the Physics system - it just needs a reference to
 * create rigid bodies when spawning objects.
 *
 * @param physics Reference to the game's Physics system
 */
Scene::Scene(Physics& physics, Renderer& renderer) : physicsWorld(physics), renderer(renderer)
, spatialGrid(std::make_unique<SpatialGrid>(10.0f))//enable by defualt
{
    std::cout << "Scene created" << std::endl;
}


/**
 * @brief Destroys the scene and cleans up all game objects.
 *
 * Calls clear() to remove all objects. The Physics system handles cleanup
 * of rigid bodies separately.
 */
Scene::~Scene() {
    clear();
}



/**
 * @brief Spawns a new game object in the scene with physics.
 *
 * This is the primary method for creating objects at runtime. It handles:
 * - Creating the physics rigid body with the specified material properties
 * - Creating the GameObject wrapper with rendering data
 * - Adding the object to the scene's management
 *
 * @param type The shape type (CUBE, SPHERE, or CAPSULE)
 * @param position World position to spawn at
 * @param size Shape dimensions (half-extents for cube, radius for sphere, radius+height for capsule)
 * @param mass Object mass in kg (0.0 = static/immovable object)
 * @param materialName Physics material name from MaterialRegistry (default: "Default")
 * @param texturePath Optional texture file path (default: "" = use default orange material)
 * @return GameObject* Pointer to the created object (owned by Scene, don't delete)
 *
 * @note The returned pointer remains valid until the object is removed or the scene is cleared.
 *
 * @example
 * // Create a dynamic wooden crate
 * auto* crate = scene.spawnObject(ShapeType::CUBE, glm::vec3(0, 5, 0),
 *                                  glm::vec3(1.0f), 10.0f, "Wood", "textures/crate.jpg");
 *
 * // Create a static ground plane
 * scene.spawnObject(ShapeType::CUBE, glm::vec3(0, 0, 0),
 *                   glm::vec3(100, 0.5f, 100), 0.0f, "Concrete");
 */
GameObject* Scene::spawnObject(ShapeType type,
    const glm::vec3& position,
    const glm::vec3& size,
    float mass,
    const std::string& materialName,
    const std::string& texturePath) 
{
    // Create physics body
    btRigidBody* body = physicsWorld.createRigidBody(
        type,
        position,
        size,
        mass,
        materialName
    );

    // Create game object 
    auto obj = std::make_unique<GameObject>(type, body, size, materialName, texturePath);
    // Store GameObject pointer in rigid body's user pointer
    // This allows raycasts to return the GameObject instead of just the raw physics body
    body->setUserPointer(obj.get());
    // Set render mesh based on shape type
    switch (type) {
    case ShapeType::CUBE:
        obj->getRender().setRenderMesh(renderer.getCubeMesh());
        break;
    case ShapeType::SPHERE:
        obj->getRender().setRenderMesh(renderer.getSphereMesh());
        break;
    case ShapeType::CAPSULE:
        obj->getRender().setRenderMesh(renderer.getCylinderMesh());
        break;
    }

    GameObject* ptr = obj.get();
    gameObjects.push_back(std::move(obj));
    // Add to spatial grid
    if (spatialGrid) {
        spatialGrid->insertObject(ptr);
    }
    // Log creation
    const char* shapeName = "Unknown";
    switch (type) {
    case ShapeType::CUBE: shapeName = "Cube"; break;
    case ShapeType::SPHERE: shapeName = "Sphere"; break;
    case ShapeType::CAPSULE: shapeName = "Capsule"; break;
    }

    std::cout << "Spawned " << shapeName << " at ("
        << position.x << ", " << position.y << ", " << position.z
        << ") with material: " << materialName << std::endl;

    return ptr;
}


// add spawnObject method without physics - for render only objects
GameObject* Scene::spawnRenderObject(
    ShapeType type,
    const glm::vec3& position,
    const glm::vec3& size,
    const std::string& texturePath)
{
    // Render-only constructor (no rigid body)
    auto obj = std::make_unique<GameObject>(type, position, size, texturePath);

    switch (type) {
    case ShapeType::CUBE:
        obj->getRender().setRenderMesh(renderer.getCubeMesh());
        break;
    case ShapeType::SPHERE:
        obj->getRender().setRenderMesh(renderer.getSphereMesh());
        break;
    case ShapeType::CAPSULE:
        obj->getRender().setRenderMesh(renderer.getCylinderMesh());
        break;
    }


    GameObject* ptr = obj.get();
    gameObjects.push_back(std::move(obj));
 

    std::cout << "Spawned Render-Only Object at ("
        << position.x << ", " << position.y << ", " << position.z << ")\n";

    return ptr;
}


void Scene::setObjectScale(GameObject* obj, const glm::vec3& newScale) {
    if (!obj) {
        std::cerr << "Error: Cannot set scale on null object" << std::endl;
        return;
    }

    // Update visual scale
    obj->getTransform().setScale(newScale);

	// If no physics, exit early - just update render scale
    if (!obj->hasPhysics()) {
        std::cout << "Updated scale for render-only object" << std::endl;
        return;
    }

    // Handle physics resize
    std::cout << "Resizing physics body for object..." << std::endl;

    btRigidBody* oldBody = obj->getRigidBody();
    if (!oldBody) {
        std::cerr << "Error: Object has physics component but no rigid body!" << std::endl;
        return;
    }

    // Get current mass
    float mass = 1.0f / oldBody->getInvMass();
    if (oldBody->getInvMass() == 0.0f) {
        mass = 0.0f;  // Static object
    }

    // Warn about constraint breakage
    auto attachedConstraints = ConstraintRegistry::getInstance().findConstraintsByObject(obj);
    if (!attachedConstraints.empty()) {
        std::cout << "WARNING: Object has " << attachedConstraints.size()
            << " attached constraint(s) which will break during resize!" << std::endl;
    }

    // Resize the rigid body
    btRigidBody* newBody = physicsWorld.resizeRigidBody(
        oldBody,
        obj->getShapeType(),
        newScale,
        mass,
        obj->getMaterialName()
    );

    // Update GameObject's physics component
    if (newBody) {
        newBody->setUserPointer(obj);
        obj->getPhysics()->setRigidBody(newBody);

        if (spatialGrid) {
            spatialGrid->updateObject(obj);
        }

        std::cout << "Object scale updated successfully" << std::endl;
    }
    else {
        std::cerr << "Error: Failed to resize rigid body!" << std::endl;
    }
}

/**
 * @brief Updates all game objects in the scene.
 *
 * Synchronizes each GameObject's position and rotation with its physics
 * rigid body state. This should be called every frame after physics simulation
 * has stepped forward.
 *
 * The update flow is:
 * 1. Physics system simulates forces and collisions
 * 2. Scene::update() syncs GameObject transforms from physics
 * 3. Renderer draws objects at their updated positions
 */
void Scene::update(EngineMode mode) {
    // Only sync transforms from physics in GAME mode
    if (mode == EngineMode::Game)
    {
        // Sync all game objects with their physics simulation
        for (auto& obj : gameObjects) {
            if (obj->hasPhysics())
                // Update each object's transform from its physics body
                obj->updateFromPhysics();
        }
    }
    else
    {
        // Editor mode: do NOT constantly overwrite gizmo transforms
        // But we still want one initial sync so objects appear
        static bool syncedOnce = false;
        if (!syncedOnce)
        {
            for (auto& obj : gameObjects)
            {
                if (obj->hasPhysics())
                    obj->updateFromPhysics();
            }
            syncedOnce = true;
        }
    }
    // Update spatial grid positions
    if (spatialGrid) {
        for (auto& obj : gameObjects) {
            spatialGrid->updateObject(obj.get());
        }
    }

    // --- Process deferred destruction ---
    if (!pendingDestroy.empty())
    {
        for (GameObject* obj : pendingDestroy)
        {
            // 1. Remove constraints
            ConstraintRegistry::getInstance().removeConstraintsForObject(obj);

            // 2. Remove from spatial grid
            if (spatialGrid)
                spatialGrid->removeObject(obj);

            // 3. Remove physics body
            if (obj->hasPhysics())
                physicsWorld.removeRigidBody(obj->getRigidBody());

            // 4. Remove from scene container
            gameObjects.erase(
                std::remove_if(gameObjects.begin(), gameObjects.end(),
                    [obj](const std::unique_ptr<GameObject>& ptr)
                    {
                        return ptr.get() == obj;
                    }),
                gameObjects.end()
            );
        }

        pendingDestroy.clear();
    }

}

// Spatial Queries

std::vector<GameObject*> Scene::findObjectsInRadius(
    const glm::vec3& center,
    float radius,
    std::function<bool(GameObject*)> filter) const
{
    // Use spatial grid if enabled - queries only nearby cells (O(k) where k = objects in range)
    if (spatialGrid) {
        return spatialGrid->queryRadius(center, radius, filter);
    }

    // Fallback: Check every object in scene (O(n) - slower but works for small scenes)
    std::vector<GameObject*> results;
    float radiusSquared = radius * radius;  // Compare squared distances 

    for (const auto& obj : gameObjects) {
        // Apply filter first if provided (e.g., only enemies, only pickups)
        if (filter && !filter(obj.get())) continue;

        // Calculate distance from center
        glm::vec3 diff = obj->getPosition() - center;
        float distSquared = glm::dot(diff, diff);  // ||v||² = v·v (cheaper than length())

        // Within radius?
        if (distSquared <= radiusSquared) {
            results.push_back(obj.get());
        }
    }

    return results;
}

GameObject* Scene::findNearestObject(
    const glm::vec3& position,
    float maxRadius,
    std::function<bool(GameObject*)> filter) const
{
    // Use spatial grid if enabled
    if (spatialGrid) {
        return spatialGrid->queryNearest(position, maxRadius, filter);
    }

    // Fallback: Check all objects and track closest
    GameObject* nearest = nullptr;
    float minDistSquared = maxRadius * maxRadius;  // Start with max search radius

    for (const auto& obj : gameObjects) {
        // Apply filter if provided
        if (filter && !filter(obj.get())) continue;

        // Calculate squared distance
        glm::vec3 diff = obj->getPosition() - position;
        float distSquared = glm::dot(diff, diff);

        // Is this closer than current best?
        if (distSquared < minDistSquared) {
            minDistSquared = distSquared;
            nearest = obj.get();
        }
    }

    return nearest;  // Returns nullptr if nothing found
}

// controls

void Scene::setSpatialGridEnabled(bool enabled) {
    // Enabling grid when it doesn't exist
    if (enabled && !spatialGrid) {
        spatialGrid = std::make_unique<SpatialGrid>(10.0f);

        // Populate grid with all existing objects
        for (const auto& obj : gameObjects) {
            spatialGrid->insertObject(obj.get());
        }
        std::cout << "Spatial grid enabled" << std::endl;
    }
    // Disabling grid when it exists
    else if (!enabled && spatialGrid) {
        spatialGrid.reset();  // Destroys grid and frees memory
        std::cout << "Spatial grid disabled" << std::endl;
    }
    // else: Already in requested state, do nothing
}

void Scene::printSpatialStats() const {
    if (spatialGrid) {
        spatialGrid->printStats();  // Show cell count, objects per cell, memory usage
    }
    else {
        std::cout << "Spatial grid is disabled" << std::endl;
    }
}
/**
 * @brief Removes all objects from the scene.
 *
 * Destroys all GameObjects and their associated physics bodies.
 * The Physics system handles cleanup of rigid bodies when GameObject
 * destructors are called.
 *
 * @note This does NOT reset the Physics world itself - only removes objects.
 */
void Scene::clear() {
    std::cout << "Clearing scene (" << gameObjects.size() << " objects)" << std::endl;
    if (spatialGrid) {
        spatialGrid->clear();
    }
    gameObjects.clear();
}

void Scene::requestDestroy(GameObject* obj)
{
    if (!obj) return;

    // Prevent double-queue
    if (std::find(pendingDestroy.begin(), pendingDestroy.end(), obj) != pendingDestroy.end())
        return;

    pendingDestroy.push_back(obj);
}



GameObject* Scene::loadAndSpawnModel(const std::string& filepath,
    const glm::vec3& position,
    const glm::vec3& scale)
{
    // Load mesh from file
    Mesh loadedMesh = Mesh::loadFromFile(filepath);

    // Check if load was successful (mesh has vertices)
    if (loadedMesh.getVertexCount() == 0) {
        std::cerr << "Failed to load model from: " << filepath << std::endl;
        return nullptr;
    }

    std::cout << "Successfully loaded model: " << filepath << std::endl;

    // Create render-only object (no physics for now)
    auto obj = std::make_unique<GameObject>(ShapeType::CUBE, position, scale, "");

    // Store the loaded mesh in the renderer's cache so it persists
    // For now, we'll use a static map to keep loaded meshes alive
    static std::unordered_map<std::string, Mesh> loadedMeshes;
    loadedMeshes[filepath] = std::move(loadedMesh);

    // Set the custom mesh
    obj->getRender().setRenderMesh(&loadedMeshes[filepath]);

    GameObject* ptr = obj.get();
    gameObjects.push_back(std::move(obj));

    std::cout << "Spawned model at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;

    return ptr;
}

