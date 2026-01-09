#include "../include/Scene.h"
#include <iostream>

Scene::Scene(Physics& physics) : physicsWorld(physics) {
    std::cout << "Scene created" << std::endl;
}

Scene::~Scene() {
    clear();
}

//  Remove when loading from files
GameObject* Scene::createCube(const glm::vec3& position, const glm::vec3& size, float mass) {
    btRigidBody* body = physicsWorld.createRigidBody(ShapeType::CUBE, position, size, mass);

    auto obj = std::make_unique<GameObject>(ShapeType::CUBE, body, size);
    GameObject* ptr = obj.get();
    gameObjects.push_back(std::move(obj));

    std::cout << "Created cube at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;

    return ptr;
}

//  Remove when loading from files
GameObject* Scene::createSphere(const glm::vec3& position, float radius, float mass) {
    btRigidBody* body = physicsWorld.createRigidBody(
        ShapeType::SPHERE,
        position,
        glm::vec3(radius),
        mass
    );

    auto obj = std::make_unique<GameObject>(ShapeType::SPHERE, body, glm::vec3(radius));
    GameObject* ptr = obj.get();
    gameObjects.push_back(std::move(obj));

    std::cout << "Created sphere at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;

    return ptr;
}
void Scene::update() {
    // Sync all game objects with their physics simulation
    for (auto& obj : gameObjects) {
        obj->updateFromPhysics();
    }
}

void Scene::clear() {
    std::cout << "Clearing scene (" << gameObjects.size() << " objects)" << std::endl;
    gameObjects.clear();
}

// TODO: Implement scene loading/saving
// void Scene::loadFromFile(const std::string& filepath) {
//     // Parse JSON/XML file
//     // Create objects based on file data
//     // Example: scene.loadFromFile("scenes/level1.json");
// }
//
// void Scene::saveToFile(const std::string& filepath) {
//     // Serialize current scene to file
//     // Save object types, positions, properties
// }