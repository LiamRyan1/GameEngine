#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>
#include "../include/Scene/GameObject.h"
#include "../include/Physics/Physics.h"
#include "../include/Physics/SpatialGrid.h" 
#include "../include/Rendering/Renderer.h"
enum class EngineMode;

class Scene {

private:
    Physics& physicsWorld;
    Renderer& renderer;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    // Spatial grid for fast proximity queries
    std::unique_ptr<SpatialGrid> spatialGrid;

    
    std::vector<GameObject*> pendingDestroy;


public:
    Scene(Physics& physics, Renderer& renderer);
    ~Scene();

    // Factory methods for creating objects
    // temporary methods - Remove when loading from files
    //GameObject* createCube(const glm::vec3& position, const glm::vec3& size, float mass, const std::string& materialName = "Default", const std::string& texturePath = "");
    //GameObject* createSphere(const glm::vec3& position, float radius, float mass, const std::string& materialName = "Default", const std::string& texturePath = "");

	//generic object creation method
    GameObject* spawnObject(ShapeType type,
        const glm::vec3& position,
        const glm::vec3& size,
        float mass,
        const std::string& materialName = "Default",
		const std::string& texturePath = "",
        const std::string & specularPath = ""
    );

    // Spawn render-only object (no physics)
    GameObject* spawnRenderObject(
        ShapeType type,
        const glm::vec3& position,
        const glm::vec3& size,
        const std::string& texturePath = "",
        const std::string & specularPath = ""
    );

    void setObjectScale(GameObject* obj, const glm::vec3& newScale);
    void setObjectPhysicsScale(GameObject* obj, const glm::vec3& newPhysicsScale);
    // Update all objects from physics simulation
    void update(EngineMode mode);

    // Get all objects for rendering
    const std::vector<std::unique_ptr<GameObject>>& getObjects() const { return gameObjects; }

	// Spatial Queries

    /**
     * Find all objects within radius.
     * @param filter Optional lambda to filter results
     */
    std::vector<GameObject*> findObjectsInRadius(
        const glm::vec3& center,
        float radius,
        std::function<bool(GameObject*)> filter = nullptr
    ) const;

    /**
     * Find nearest object within max radius.
     */
    GameObject* findNearestObject(
        const glm::vec3& position,
        float maxRadius,
        std::function<bool(GameObject*)> filter = nullptr
    ) const;

    // === Spatial Grid Control ===

    void setSpatialGridEnabled(bool enabled);
    bool isSpatialGridEnabled() const { return spatialGrid != nullptr; }
    void printSpatialStats() const;

    
    void requestDestroy(GameObject* obj);


    // Clear all objects
    void clear();

    // Scene serialization
    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);


    /**
     * @brief Load .obj model and spawn it in the scene
     * @param filepath Path to .obj file
     * @param position Spawn position
     * @param meshSscale  Visual mesh  scale
     * @param enablePhysics Whether to add physics
     * @param mass Mass for rigid body (0 = static)
     * @param physicsBoxScale Scale factor for collision box (0.9 = 90% of model size)
     * @param materialName Physics material name
     * @return Pointer to spawned GameObject
     */
    GameObject* loadAndSpawnModel(const std::string& filepath,
        const glm::vec3& position,
        const glm::vec3& meshScale,
        bool enablePhysics,
        float mass,
        const glm::vec3& physicsBoxScale,
        const std::string& materialName
    );

   

};

#endif