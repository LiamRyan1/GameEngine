#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>
#include "../include/GameObject.h"
#include "../include/Physics.h"

class Scene {

private:
    Physics& physicsWorld;
    std::vector<std::unique_ptr<GameObject>> gameObjects;

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

    // Update all objects from physics simulation
    void update();

    // Get all objects for rendering
    const std::vector<std::unique_ptr<GameObject>>& getObjects() const { return gameObjects; }

    // Clear all objects
    void clear();

    // TODO: Add scene loading from files
    // void loadFromFile(const std::string& filepath);
    // void saveToFile(const std::string& filepath);


};

#endif