#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H


#include <memory>
#include <string>
#include <vector>
#include <utility>   
#include "../include/Scene/TransformComponent.h"
#include "../include/Scene/PhysicsComponent.h"
#include "../include/Scene/RenderComponent.h"
#include "../include/Scene/ScriptComponent.h"
/**
 * @brief  A component-based game object.
 *
 * GameObject is now a lightweight container for components:
 *  TransformComponent (always present) - position, rotation, scale
 * - RenderComponent (always present) - shape, texture
 * - PhysicsComponent (optional) - rigid body, physics simulation
 *  - ScriptComponent(s) (optional) - gameplay logic (multiple allowed)
 * Usage examples:
 * //adding scripts to game objects
 *  player->addScript<PlayerController>(&camera);
 *  player->addScript<HealthComponent>(100.0f);
 * // Physics-enabled object (typical game object)
 * auto crate = GameObject(ShapeType::CUBE, body, glm::vec3(1.0f), "Wood", "crate.jpg");
 *
 * // Render-only object (particle, decoration)
 * auto particle = GameObject(ShapeType::SPHERE, glm::vec3(0,5,0), glm::vec3(0.1f), "particle.png");
 */
class GameObject {
private:
	// scale for physics collision shape - separate from visual scale to allow for non-uniform scaling without affecting physics
    glm::vec3 physicsScale;
    static uint64_t nextID;
    uint64_t id;

    std::string name; //create a unique name for each object - this needs to be implemented 
    // Core components (always present)
    TransformComponent transform;
    RenderComponent render;

    // Optional components
    std::unique_ptr<PhysicsComponent> physics;
	// Support multiple scripts per object - store in a vector
    std::vector<std::unique_ptr<ScriptComponent>> scripts;

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
        *@brief Attach a script to this object.
        *
        * Constructs the script in - place, sets its owner, then calls onStart().
        * You can attach multiple scripts of different types.
        *
        * @tparam T   Must derive from ScriptComponent
        * @tparam Args Constructor argument types(deduced)
        * @return Raw pointer to the newly attached script(owned by this GameObject)
        *
        *@example
        * player->addScript<PlayerController>(&camera);
        *player->addScript<HealthComponent>(100.0f);
    */
    template<typename T, typename... Args>
    T * addScript(Args&&... args)
    {
        auto script = std::make_unique<T>(std::forward<Args>(args)...);
        script->setOwner(this);
        script->onStart();
        T* raw = script.get();
        scripts.push_back(std::move(script));
        return raw;
    }
    /** Called by Scene::update() every frame in Game mode */
    void updateScripts(float dt);

    /** Called by Scene::update() every physics tick in Game mode */
    void fixedUpdateScripts(float fixedDt);

    /** Calls onDestroy() on all scripts - called by Scene before destruction */
    void notifyDestroy();


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

    glm::vec3 getPhysicsScale() const { return physicsScale; }
    void setPhysicsScale(const glm::vec3& scale);

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