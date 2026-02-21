#include <GL/glew.h> 
#include "../include/Scene/Scene.h"
#include "../include/Core/Engine.h"
#include "../include/Physics/ConstraintRegistry.h"
#include "../include/Rendering/MeshFactory.h"
#include <unordered_map> 
#include <iostream>
#include <algorithm>  // std::min, std::max
#include <cfloat>  // for FLT_MAX
#include "../External/json/json.hpp"
using json = nlohmann::json;
#include <fstream>
#include <filesystem>


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
    const std::string& texturePath,
    const std::string& specularPath)
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
    
    // Set specular texture if provided
    if (!specularPath.empty()) {
        obj->getRender().setSpecularTexturePath(specularPath);
    }
    
    // Store GameObject pointer in rigid body's user pointer
    // This allows raycasts to return the GameObject instead of just the raw physics body
    body->setUserPointer(obj.get());
    obj->updateFromPhysics();

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
    const std::string& texturePath,
    const std::string& specularPath)
{
    // Render-only constructor (no rigid body)
    auto obj = std::make_unique<GameObject>(type, position, size, texturePath);

    // Set specular texture if provided
    if (!specularPath.empty()) {
        obj->getRender().setSpecularTexturePath(specularPath);
    }

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

    json test;
    test["hello"] = "world";

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

    glm::vec3 physicsScale = obj->getPhysicsScale();
    glm::vec3 collisionSize = newScale * physicsScale;
    // Resize the rigid body
    btRigidBody* newBody = physicsWorld.resizeRigidBody(
        oldBody,
        obj->getShapeType(),
        collisionSize,
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
    const glm::vec3& meshScale, 
    bool enablePhysics,
    float mass,
    const glm::vec3& physicsBoxScale,
    const std::string& materialName)
{
    // Load mesh from file
    Mesh loadedMesh = MeshFactory::loadFromFile(filepath);

    // Check if load was successful (mesh has vertices)
    if (loadedMesh.getVertexCount() == 0) {
        std::cerr << "Failed to load model from: " << filepath << std::endl;
        return nullptr;
    }

    std::cout << "Successfully loaded model: " << filepath << std::endl;

    // Store mesh in static cache
    static std::unordered_map<std::string, Mesh> loadedMeshes;
    loadedMeshes[filepath] = std::move(loadedMesh);
    Mesh* meshPtr = &loadedMeshes[filepath];

    GameObject* obj = nullptr;
 
    if (enablePhysics) {

        // Calculate bounding box from mesh vertices
        const std::vector<float>& verts = meshPtr->getVertices();
        const size_t FLOATS_PER_VERTEX = 6;
        glm::vec3 minBounds(FLT_MAX);
        glm::vec3 maxBounds(-FLT_MAX);

        // Iterate through vertices (skip normals, only read positions)
        for (size_t i = 0; i < verts.size(); i += FLOATS_PER_VERTEX) {
            glm::vec3 pos(verts[i], verts[i + 1], verts[i + 2]);

            minBounds.x = std::min(minBounds.x, pos.x);
            minBounds.y = std::min(minBounds.y, pos.y);
            minBounds.z = std::min(minBounds.z, pos.z);

            maxBounds.x = std::max(maxBounds.x, pos.x);
            maxBounds.y = std::max(maxBounds.y, pos.y);
            maxBounds.z = std::max(maxBounds.z, pos.z);
        }

        std::cout << "Model bounds: min(" << minBounds.x << "," << minBounds.y << "," << minBounds.z
            << ") max(" << maxBounds.x << "," << maxBounds.y << "," << maxBounds.z << ")" << std::endl;

        // Calculate collision box size and center offset
        glm::vec3 boundingBoxSize = (maxBounds - minBounds) * physicsBoxScale;
        glm::vec3 halfExtents = boundingBoxSize * 0.5f;

        // Calculate where the center of the bounding box is relative to model origin
        glm::vec3 boundingBoxCenter = (minBounds + maxBounds) * 0.5f * meshScale;

        // Adjust spawn position to account for model's offset
        glm::vec3 adjustedPosition = position + boundingBoxCenter;

        std::cout << "Creating physics box: " << halfExtents.x << ", "
            << halfExtents.y << ", " << halfExtents.z << std::endl;
        std::cout << "Bounding box center offset: " << boundingBoxCenter.x << ", "
            << boundingBoxCenter.y << ", " << boundingBoxCenter.z << std::endl;

        // Create rigid body at adjusted position
        btRigidBody* body = physicsWorld.createRigidBody(
            ShapeType::CUBE,
            adjustedPosition,  // Use adjusted position
            halfExtents,
            mass,
            materialName
        );

        // Create GameObject with physics
        auto objUnique = std::make_unique<GameObject>(
            ShapeType::CUBE, body, halfExtents, materialName, ""
        );

        // Store physicsBoxScale
        objUnique->setPhysicsScale(physicsBoxScale);

        body->setUserPointer(objUnique.get());
        objUnique->getRender().setRenderMesh(meshPtr);
        objUnique->setPosition(adjustedPosition);  // Use adjusted position
        objUnique->setScale(meshScale);

        obj = objUnique.get();
        gameObjects.push_back(std::move(objUnique));

        if (spatialGrid) {
            spatialGrid->insertObject(obj);
        }

        std::cout << "Spawned model with physics at (" << adjustedPosition.x << ", "
            << adjustedPosition.y << ", " << adjustedPosition.z << ")" << std::endl;
    }
    else {
        // Create render-only object
        auto objUnique = std::make_unique<GameObject>(
            ShapeType::CUBE, position, meshScale, ""
        );

        objUnique->getRender().setRenderMesh(meshPtr);

        obj = objUnique.get();
        gameObjects.push_back(std::move(objUnique));

        std::cout << "Spawned render-only model at (" << position.x << ", "
            << position.y << ", " << position.z << ")" << std::endl;
    }
    return obj;
}

void Scene::setObjectPhysicsScale(GameObject* obj, const glm::vec3& newPhysicsScale) {
    if (!obj || !obj->hasPhysics()) {
        std::cerr << "Error: Cannot set physics scale on object without physics" << std::endl;
        return;
    }

    std::cout << "Resizing physics collision shape..." << std::endl;

    btRigidBody* oldBody = obj->getRigidBody();
    if (!oldBody) {
        std::cerr << "Error: No rigid body found!" << std::endl;
        return;
    }

    // Get current properties
    float mass = 1.0f / oldBody->getInvMass();
    if (oldBody->getInvMass() == 0.0f) {
        mass = 0.0f;  // Static object
    }

    // Get base collision shape size (from original scale, not current physics scale)
    glm::vec3 baseScale = obj->getScale();

    // Calculate new collision shape size
    glm::vec3 newCollisionSize = baseScale * newPhysicsScale;

    // Resize the rigid body
    btRigidBody* newBody = physicsWorld.resizeRigidBody(
        oldBody,
        obj->getShapeType(),
        newCollisionSize,
        mass,
        obj->getMaterialName()
    );

    if (newBody) {
        newBody->setUserPointer(obj);
        obj->getPhysics()->setRigidBody(newBody);
        obj->setPhysicsScale(newPhysicsScale);

        if (spatialGrid) {
            spatialGrid->updateObject(obj);
        }

        std::cout << "Physics scale updated successfully" << std::endl;
    }
    else {
        std::cerr << "Error: Failed to resize rigid body!" << std::endl;
    }
}

bool Scene::saveToFile(const std::string& path) const
{
    json sceneJson;
    sceneJson["objects"] = json::array();

    for (const auto& objPtr : gameObjects)
    {
        const GameObject& obj = *objPtr;

        json o;

        // Basic info
        o["id"] = obj.getID();
        o["name"] = obj.getName();
        o["shape"] = (int)obj.getShapeType();

        // Transform
        o["transform"]["position"] = {
            obj.getPosition().x,
            obj.getPosition().y,
            obj.getPosition().z
        };

        o["transform"]["rotation"] = {
            obj.getRotation().x,
            obj.getRotation().y,
            obj.getRotation().z,
            obj.getRotation().w
        };

        o["transform"]["scale"] = {
            obj.getScale().x,
            obj.getScale().y,
            obj.getScale().z
        };

        // Render
        o["render"]["texture"] = obj.getTexturePath();

        // Physics
        o["physics"]["enabled"] = obj.hasPhysics();
        o["physics"]["physicsScale"] = {
            obj.getPhysicsScale().x,
            obj.getPhysicsScale().y,
            obj.getPhysicsScale().z
        };

        if (obj.hasPhysics())
        {
            btRigidBody* rb = obj.getRigidBody();

            float mass = 0.0f;
            if (rb->getInvMass() != 0.0f)
                mass = 1.0f / rb->getInvMass();

            o["physics"]["mass"] = mass;
            o["physics"]["material"] = obj.getMaterialName();
        }

        sceneJson["objects"].push_back(o);
    }
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());

    std::ofstream file(path);
    if (!file.is_open())
    {
        std::cout << "Failed to save scene: " << path << std::endl;
        return false;
    }

    file << sceneJson.dump(4);

    std::cout << "Scene saved to " << path << std::endl;
    return true;
}

bool Scene::loadFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "Failed to load scene: " << path << std::endl;
        return false;
    }

    json sceneJson;
    file >> sceneJson;

    // Remove existing objects
    clear();

    for (const auto& o : sceneJson["objects"])
    {
        ShapeType shape = (ShapeType)o["shape"].get<int>();

        glm::vec3 position(
            o["transform"]["position"][0],
            o["transform"]["position"][1],
            o["transform"]["position"][2]
        );

        glm::quat rotation(
            o["transform"]["rotation"][3],
            o["transform"]["rotation"][0],
            o["transform"]["rotation"][1],
            o["transform"]["rotation"][2]
        );

        glm::vec3 scale(
            o["transform"]["scale"][0],
            o["transform"]["scale"][1],
            o["transform"]["scale"][2]
        );

        std::string texture = o["render"]["texture"];

        bool physicsEnabled = o["physics"]["enabled"];

        GameObject* obj = nullptr;

        if (physicsEnabled)
        {
            float mass = o["physics"]["mass"];
            std::string material = o["physics"]["material"];

            glm::vec3 physScale(
                o["physics"]["physicsScale"][0],
                o["physics"]["physicsScale"][1],
                o["physics"]["physicsScale"][2]
            );

            glm::vec3 collisionSize = scale * physScale;

            obj = spawnObject(shape, position, collisionSize, mass, material, texture);

            obj->setScale(scale);
            obj->setPhysicsScale(physScale);

            // force exact transform into Bullet immediately
            obj->getPhysics()->syncFromTransform(obj->getTransform());

            // make render match physics immediately
            obj->updateFromPhysics();
        }

        else
        {
            obj = spawnRenderObject(shape, position, scale, texture);
        }

        obj->setRotation(rotation);

        if (o.contains("name"))
            obj->setName(o["name"]);
    }

    std::cout << "Scene loaded from " << path << std::endl;

    // --- Apply exact transforms to physics bodies (NO simulation) ---
    for (auto& obj : gameObjects)
    {
        if (obj->hasPhysics())
        {
            // Copy transform -> rigid body
            obj->getPhysics()->syncFromTransform(obj->getTransform());
        }
    }

    // --- Freeze everything so nothing moves ---
    for (auto& obj : gameObjects)
    {
        if (obj->hasPhysics())
        {
            btRigidBody* body = obj->getRigidBody();
            if (body)
            {
                body->setLinearVelocity(btVector3(0, 0, 0));
                body->setAngularVelocity(btVector3(0, 0, 0));
                body->clearForces();
                body->setActivationState(ISLAND_SLEEPING);
            }
        }
    }

    return true;
}


