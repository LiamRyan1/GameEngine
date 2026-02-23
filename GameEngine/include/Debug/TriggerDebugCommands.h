#pragma once
#include <functional>
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Trigger;
class GameObject;
enum class TriggerType;

struct TriggerDebugCommands
{
    // Create triggers
    std::function<Trigger* (const std::string& name, TriggerType type,
        const glm::vec3& position, const glm::vec3& size)> createTrigger;

    // Remove triggers
    std::function<void(Trigger*)> removeTrigger;
    std::function<bool(const std::string& name)> removeTriggerByName;
    std::function<void()> clearAllTriggers;

    // Query triggers
    std::function<Trigger* (const std::string& name)> findTriggerByName;
    std::function<std::vector<Trigger*>(TriggerType)> findTriggersByType;
    std::function<std::vector<Trigger*>(GameObject*)> findTriggersContainingObject;

    // Teleport-specific
    std::function<void(Trigger*, const glm::vec3&)> setTeleportDestination;

    // Speed zone-specific
    std::function<void(Trigger*, const glm::vec3&, float)> setForce;

    // Update trigger transform
    std::function<void(Trigger*, const glm::vec3& position)> updateTriggerPosition;
    std::function<void(Trigger*, const glm::vec3& size)> updateTriggerSize;

    // Enable/disable triggers
    std::function<void(Trigger*, bool enabled)> setTriggerEnabled;
};