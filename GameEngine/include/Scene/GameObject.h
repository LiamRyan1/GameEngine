#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H


#include <memory>
#include <string>
#include "../include/Scene/TransformComponent.h"
#include "../include/Scene/PhysicsComponent.h"
#include "../include/Scene/RenderComponent.h"

/**
 * @brief  A component-based game object.
 *
 * GameObject is now a lightweight container for components:
 *  TransformComponent (always present) - position, rotation, scale
 * - RenderComponent (always present) - shape, texture
 * - PhysicsComponent (optional) - rigid body, physics simulation
 * Usage examples:
 *
 * // Physics-enabled object (typical game object)
 * auto crate = GameObject(ShapeType::CUBE, body, glm::vec3(1.0f), "Wood", "crate.jpg");
 *
 * // Render-only object (particle, decoration)
 * auto particle = GameObject(ShapeType::SPHERE, glm::vec3(0,5,0), glm::vec3(0.1f), "particle.png");
 */
class GameObject {
private:

    static uint64_t nextID;
    uint64_t id;

    std::string name; //create a unique name for each object - this needs to be implemented 
    // Core components (always present)
    TransformComponent transform;
    RenderComponent render;

    // Optional components
    std::unique_ptr<PhysicsComponent> physics;

public:

    // Getter for ID
    uint64_t getID() const { return id; }

    /**
     * @brief Constructs a GameObject with physics and rendering properties.
     * @param type Collision shape type
     * @param body Bullet rigid body pointer (can be nullptr for render-only)
     * @param scale Object dimensions
     * @param materialName Physics material from registry (default: "Default")
     * @param texturePath Path to texture file (default: "" = orange color)
     */
    GameObject(ShapeType type, btRigidBody* body, const glm::vec3& scale,
        const std::string& materialName = "Default",
        const std::string& texturePath = "");

    /**
     * @brief Creates a GameObject WITHOUT physics (render-only).
     * Use this for visual effects, decorations, UI elements, particles.
     *
     * @param type Shape type for rendering
     * @param position Initial world position
     * @param scale Object dimensions
     * @param texturePath Texture file path (empty = default orange)
     */
    GameObject(ShapeType type, const glm::vec3& position,
        const glm::vec3& scale, const std::string& texturePath = "");

    ~GameObject();



    /**
     * @brief Updates transform from physics simulation.
     *
     * Pulls position and rotation from the rigid body's motion state.
     * Call this every frame after physics has stepped.
     */
    void updateFromPhysics();

    // Component accessors - allows direct access to components
    TransformComponent& getTransform() { return transform; }
    const TransformComponent& getTransform() const { return transform; }

    RenderComponent& getRender() { return render; }
    const RenderComponent& getRender() const { return render; }

    PhysicsComponent* getPhysics() { return physics.get(); }
    const PhysicsComponent* getPhysics() const { return physics.get(); }

    bool hasPhysics() const { return physics != nullptr; }
    bool isRenderOnly() const { return !hasPhysics(); }



    // Transform shortcuts
    glm::vec3 getPosition() const { return transform.getPosition(); }
    glm::quat getRotation() const { return transform.getRotation(); }
    glm::vec3 getScale() const { return transform.getScale(); }

    void setPosition(const glm::vec3& pos);
    void setRotation(const glm::quat& rot);
    void setScale(const glm::vec3& scale);

    // Render shortcuts
    ShapeType getShapeType() const { return render.getShapeType(); }
    const std::string& getTexturePath() const { return render.getTexturePath(); }
    void setTexturePath(const std::string& path) { render.setTexturePath(path); }

    // Name
    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }

    // Physics shortcuts
    btRigidBody* getRigidBody() const {
        return physics ? physics->getRigidBody() : nullptr;
    }
    std::string getMaterialName() const {
        return physics ? physics->getMaterialName() : "";
    }
};
#endif // GAMEOBJECT_H