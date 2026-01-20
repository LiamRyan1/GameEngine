#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <btBulletDynamicsCommon.h>
#include <memory>

class Mesh;

/**
 * @brief Defines the collision shape types available for game objects.
 *
 * These map to Bullet Physics primitive shapes and determine both
 * physics collision behavior and rendering appearance.
 */
enum class ShapeType {
    CUBE,
    SPHERE,
    CAPSULE,
};

/**
 * @brief Represents a game object with physics, transform, and rendering data.
 *
 * GameObject is a lightweight wrapper that connects three systems:
 * 1. Physics: Wraps a Bullet rigid body for simulation
 * 2. Transform: Caches position/rotation/scale for rendering
 * 3. Rendering: Stores material name and texture path
 *
 * Design principles:
 * - GameObject does NOT own the rigid body (Physics system owns it)
 * - Transform is cached and synced from physics each frame
 * - Minimal data - just enough to render and identify the object
 *
 * Lifecycle:
 * 1. Created by Scene::spawnObject()
 * 2. Updated every frame via updateFromPhysics()
 * 3. Destroyed when Scene is cleared or object is removed
 */
class GameObject {
private:
	std::string name; //create a unique name for each object - this needs to be implemented 
    ShapeType shapeType;
	btRigidBody* rigidBody; 

    glm::vec3 position;
    glm::quat rotation;

    glm::vec3 scale;
    std::string texturePath;
    std::string materialName;

public:
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
    ~GameObject();



    /**
     * @brief Updates transform from physics simulation.
     *
     * Pulls position and rotation from the rigid body's motion state.
     * Call this every frame after physics has stepped.
     */
    void updateFromPhysics();

	// Getters

    /** @brief Gets the collision shape type. */
    ShapeType getShapeType() const { return shapeType; }

    /** @brief Gets the current world position. */
    glm::vec3 getPosition() const { return position; }

    /** @brief Gets the current world rotation as a quaternion. */
    glm::quat getRotation() const { return rotation; }

    /** @brief Gets the object scale (render size).*/
    glm::vec3 getScale() const { return scale; }

    /** @brief Gets the physics material name. */
    std::string getMaterialName() const { return materialName; }

    /** @brief Gets the Bullet rigid body pointer (can be nullptr). */
    btRigidBody* getRigidBody() const { return rigidBody; }

    /** @brief Gets the texture file path (empty string = default color). */
    const std::string& getTexturePath() const { return texturePath; }

    /**
    * @brief Sets the texture path for rendering.
    * @param path Path to texture file (e.g. "textures/wood.jpg")
    * @note Does not reload texture - renderer handles that
    */
    void setTexturePath(const std::string& path) { texturePath = path; }

    /**
     * @brief Sets the physics material name.
     * @param name Material name from MaterialRegistry
     * @warning Currently does not update the rigid body's friction/restitution.
     *          Material is only used during object creation. To change physics
     *          properties at runtime, you'd need to modify the rigid body directly.
     * @todo Add method to apply material changes to existing rigid body
     */
    void setMaterialName(const std::string& name) { materialName = name; }

    /**
     * @brief Sets the object's world position.
     *
     * Updates both cached position AND physics rigid body (if present).
     * Wakes up the rigid body to ensure collision detection works.
     *
     * @param newPos New world position
     */
    void setPosition(const glm::vec3& newPos);

     /**
     * @brief Sets the object's render scale.
     * 
     * @param newScale New scale vector
     * @warning Currently only affects rendering/picking, NOT physics collision shape.
     * @todo Implement physics shape resizing
     */
    void setScale(const glm::vec3& newScale);

    /**
	 * @brief Sets the object's world rotation.
	 * @param newRot New rotation as a quaternion
	 * @note Updates both cached rotation AND physics rigid body (if present).
	 * @todo implement this function
     */
    void setRotation(const glm::quat& newRot);
};
#endif // GAMEOBJECT_H