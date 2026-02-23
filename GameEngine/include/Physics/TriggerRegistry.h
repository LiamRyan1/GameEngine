#ifndef TRIGGER_REGISTRY_H
#define TRIGGER_REGISTRY_H

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <btBulletDynamicsCommon.h>

class Trigger;
enum class TriggerType;
class GameObject;

/**
 * @brief Singleton registry that manages all triggers in the scene.
 *
 * Responsibilities:
 * - Creating and destroying triggers
 * - Adding/removing triggers from physics world
 * - Updating all triggers each frame
 * - Querying triggers by name, type, or location
 * - Cleanup when scene is destroyed
 */
class TriggerRegistry {
private:
    static TriggerRegistry* instance;

    btDiscreteDynamicsWorld* dynamicsWorld;
    std::vector<std::unique_ptr<Trigger>> triggers;

    TriggerRegistry();

public:
    ~TriggerRegistry();

    // Singleton access
    static TriggerRegistry& getInstance();

    /**
     * @brief Initialize with physics world reference
     * Must be called before using triggers
     */
    void initialize(btDiscreteDynamicsWorld* world);

    // === Trigger Creation ===

    /**
     * @brief Create a new trigger and add it to the world
     * @param name Display name
     * @param type Trigger behavior type
     * @param position World position (center)
     * @param size Dimensions (half-extents)
     * @return Pointer to created trigger (owned by registry)
     */
    Trigger* createTrigger(const std::string& name,
        TriggerType type,
        const glm::vec3& position,
        const glm::vec3& size);

    // === Trigger Management ===

    /**
     * @brief Add an existing trigger to the registry
     * @return Raw pointer to the trigger (ownership transferred)
     */
    Trigger* addTrigger(std::unique_ptr<Trigger> trigger);

    /**
     * @brief Remove and destroy a trigger
     */
    void removeTrigger(Trigger* trigger);

    /**
     * @brief Remove trigger by name
     */
    bool removeTrigger(const std::string& name);

    /**
     * @brief Remove all triggers
     */
    void clearAll();

    // === Update ===

    /**
     * @brief Update all triggers (check for enter/exit events)
     * Should be called every physics step
     */
    void update(float deltaTime);

    // === Queries ===

    /**
     * @brief Find trigger by name
     */
    Trigger* findTriggerByName(const std::string& name) const;

    /**
     * @brief Get all triggers of a specific type
     */
    std::vector<Trigger*> findTriggersByType(TriggerType type) const;

    /**
     * @brief Get all triggers
     */
    std::vector<Trigger*> getAllTriggers() const;

    /**
     * @brief Check if trigger with name exists
     */
    bool hasTrigger(const std::string& name) const;

    /**
     * @brief Get number of active triggers
     */
    size_t getTriggerCount() const { return triggers.size(); }

    /**
     * @brief Find all triggers within radius of a point
     */
    std::vector<Trigger*> findTriggersInRadius(const glm::vec3& position, float radius) const;

    /**
     * @brief Find what trigger (if any) contains a point
     */
    Trigger* findTriggerContainingPoint(const glm::vec3& point) const;

    /**
     * @brief Check if an object is inside any trigger
     */
    std::vector<Trigger*> findTriggersContainingObject(GameObject* obj) const;

    // === Debug ===

    /**
     * @brief Print statistics about all triggers
     */
    void printStats() const;

    /**
     * @brief Print detailed info about all triggers
     */
    void printAllTriggers() const;

private:
    void addToPhysicsWorld(Trigger* trigger);
    void removeFromPhysicsWorld(Trigger* trigger);
};

#endif // TRIGGER_REGISTRY