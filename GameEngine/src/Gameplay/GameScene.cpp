#include "../include/Gameplay/GameScene.h"
#include "../include/Gameplay/PlayerController.h"
#include "../include/Scene/Scene.h"
#include "../include/Rendering/Camera.h"
#include "../include/Physics/Physics.h"

void SetupGameScene(Scene& scene, Camera& camera, Physics& physics)
{
    // Ground
    scene.spawnObject(ShapeType::CUBE, glm::vec3(0, -0.25f, 0), glm::vec3(100.0f, 0.5f, 100.0f), 0.0f, "Default");

    // Player
    GameObject* player = scene.spawnObject(
        ShapeType::CAPSULE,
        glm::vec3(0.0f, 3.0f, 0.0f),
        glm::vec3(0.5f, 1.0f, 0.5f),
        1.0f, "Default"
    );
    player->setName("Player");
    player->getRigidBody()->setFriction(0.8f);
    player->getRigidBody()->setRestitution(0.0f);
    player->getRigidBody()->setContactProcessingThreshold(0.0f);
    // Pass PhysicsQuery so PlayerController can do proper ground raycasts
    player->addScript<PlayerController>(&camera, &physics.getQuerySystem());
}
void SetupScripts(Scene& scene, Camera& camera, Physics& physics)
{
    // Find the player by name and re-attach its script
    for (auto& obj : scene.getObjects())
    {
        if (obj->getName() == "Player")
        {
            obj->addScript<PlayerController>(&camera, &physics.getQuerySystem());
        }
        // Add more script re-attachments here as game grows
        // e.g. if (obj->getName() == "Enemy") obj->addScript<EnemyAI>();
    }
}