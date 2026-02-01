#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>
#include "../include/GameObject.h"
#include "../include/Physics.h"
#include "../include/SpatialGrid.h" 
enum class EngineMode;

class Scene {

private:
    Physics& physicsWorld;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    // Spatial grid for fast proximity queries
    std::unique_ptr<SpatialGrid> spatialGrid;

public:
    Scene(Physics& physics);
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
		const std::string& texturePath = "");

    // Spawn render-only object (no physics)
    GameObject* spawnRenderObject(
        ShapeType type,
        const glm::vec3& position,
        const glm::vec3& size,
        const std::string& texturePath = ""
    );

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

    // Clear all objects
    void clear();

    // TODO: Add scene loading from files
    // void loadFromFile(const std::string& filepath);
    // void saveToFile(const std::string& filepath);


};

#endif