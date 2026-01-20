#include "../include/GameObject.h"


/**
 * @brief Constructs a new GameObject with physics and rendering data.
 *
 * Creates a game object that wraps a Bullet Physics rigid body and stores
 * rendering information like shape, texture, and material. The GameObject
 * does not own the rigid body - it's managed by the Physics system.
 *
 * @param type The collision shape type (CUBE, SPHERE, or CAPSULE)
 * @param body Pointer to the Bullet rigid body (owned by Physics system, can be nullptr for render-only objects)
 * @param scale Object dimensions (half-extents for cubes, radius for spheres, radius+height for capsules)
 * @param materialName Physics material name for friction/restitution properties (default: "Default")
 * @param texturePath Path to texture file relative to executable (default: "" = use default orange color)
 *
 * @note Position and rotation are initialized to origin and identity - they get updated from
 *       physics on the first updateFromPhysics() call.
 */
GameObject::GameObject(ShapeType type, btRigidBody* body, const glm::vec3& scale, const std::string& materialName,
    const std::string& texturePath)
    : shapeType(type), rigidBody(body), scale(scale),
    materialName(materialName), texturePath(texturePath),
    position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f)
     {
}


/**
 * @brief Destroys the GameObject.
 *
 * The GameObject does not own its rigid body - physics cleanup is handled
 * by the Physics system when it removes the body from the dynamics world.
 */
GameObject::~GameObject() {
    // Physics cleanup handled by Physics system
}



/**
 * @brief Synchronizes GameObject transform with its physics rigid body.
 *
 * Pulls the current position and rotation from the Bullet physics simulation
 * and updates the GameObject's cached transform. This should be called every
 * frame after physics simulation has stepped.
 *
 * The flow is:
 * 1. Physics system simulates forces, collisions, constraints
 * 2. updateFromPhysics() reads the new transform from Bullet
 * 3. Renderer uses the updated position/rotation to draw the object
 *
 * @note Does nothing if rigidBody is nullptr (for render-only objects without physics)
 * @note Uses the motion state to get interpolated transforms for smooth rendering
 */
void GameObject::updateFromPhysics() {
	// Ensure the rigid body and its motion state are valid
    if (!rigidBody) return;
	// Get the world transform from the rigid body's motion state
    btTransform trans;
    rigidBody->getMotionState()->getWorldTransform(trans);

	// Update position and rotation from the transform
    // Extract position
    btVector3 origin = trans.getOrigin();
    position = glm::vec3(origin.x(), origin.y(), origin.z());

    // Extract rotation
    btQuaternion rot = trans.getRotation();
    rotation = glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
}

/**
 * @brief Sets the object's position in world space.
 *
 * Updates both the cached position AND the physics rigid body (if present).
 * This is essential because physics simulation will override any changes
 * that aren't also written to the rigid body.
 *
 * Use cases:
 * - Editor: Moving objects with the transform gizmo
 * - Gameplay: Teleporting player/objects to spawn points
 * - Respawn: Resetting objects to initial positions
 *
 * @param newPos The new world position
 *
 * @note If the object has physics, this wakes up the rigid body so the change
 *       takes effect immediately and the object participates in collision detection.
 * @note Preserves the current rotation - only position is changed.
 */
void GameObject::setPosition(const glm::vec3& newPos)
{
    position = newPos;

    if (rigidBody)
    {
        btTransform trans;
        trans.setIdentity();

        // Keep current rotation
        trans.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
        trans.setOrigin(btVector3(newPos.x, newPos.y, newPos.z));

        rigidBody->setWorldTransform(trans);

        if (rigidBody->getMotionState())
            rigidBody->getMotionState()->setWorldTransform(trans);

        // Wake up so the change takes effect immediately
        rigidBody->activate(true);
    }
}



/**
 * @brief Sets the object's scale for rendering and editor picking.
 *
 * Updates the visual size of the object. Currently this only affects rendering
 * and ray-casting picking in the editor - it does NOT resize the physics
 * collision shape.
 *
 * @param newScale The new scale (typically uniform, e.g. glm::vec3(2.0f) for 2x size)
 *
 * @warning This does NOT update the Bullet collision shape yet. The physics collider
 *          will remain at its original size. This will be fixed in a future update
 *          by recreating the collision shape with new dimensions.
 *
 * @todo Implement collision shape resizing by:
 *       1. Getting the current shape type and properties
 *       2. Creating a new shape with scaled dimensions
 *       3. Replacing the old shape on the rigid body
 *       4. Cleaning up the old shape
 */
void GameObject::setScale(const glm::vec3& newScale)
{
    scale = newScale;
}