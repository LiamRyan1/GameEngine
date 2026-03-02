#ifndef FORCE_GENERATOR_REGISTRY_H
#define FORCE_GENERATOR_REGISTRY_H

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

class ForceGenerator;
enum class ForceGeneratorType;
//singleton registry that manages all force generators in the scene
// its responsibilities include: registering new generators, updating all generators each frame, removing expired generators, and providing query methods to find generators by name, type, etc.
class ForceGeneratorRegistry {
private:
    static ForceGeneratorRegistry* instance;

    btDiscreteDynamicsWorld* dynamicsWorld = nullptr;
    std::vector<std::unique_ptr<ForceGenerator>> generators;

    ForceGeneratorRegistry() = default;

public:
    ~ForceGeneratorRegistry();

    static ForceGeneratorRegistry& getInstance();

    /**
     * @brief Initialize with the physics world.
     * Must be called before adding generators or calling update().
     */
    void initialize(btDiscreteDynamicsWorld* world);
    

    /**
     * @brief Add a generator to the registry.
     * @return Raw pointer to the generator (owned by registry).
     */
    ForceGenerator* addGenerator(std::unique_ptr<ForceGenerator> generator);

    /**
     * @brief Remove a generator by pointer.
     */
    void removeGenerator(ForceGenerator* generator);

    /**
     * @brief Remove a generator by name.
     */
    bool removeGenerator(const std::string& name);

    /**
     * @brief Remove all generators.
     */
    void clearAll();


	// factory methods for creating common generator types (also add to registry)
    /**
     * @brief Create and add a wind zone.
     */
    ForceGenerator* createWind(const std::string& name,
        const glm::vec3& position,
        float radius,
        const glm::vec3& direction,
        float strength);

    /**
     * @brief Create and add a gravity well.
     * Positive strength = attract, negative = repel.
     */
    ForceGenerator* createGravityWell(const std::string& name,
        const glm::vec3& position,
        float radius,
        float strength);

    /**
     * @brief Create and add a vortex.
     */
    ForceGenerator* createVortex(const std::string& name,
        const glm::vec3& position,
        float radius,
        const glm::vec3& axis,
        float rotationStrength,
        float pullStrength);

    /**
     * @brief Spawn a one-shot explosion that fires this frame then auto-removes.
     */
    ForceGenerator* createExplosion(const std::string& name,
        const glm::vec3& position,
        float radius,
        float strength);
	// updates all generators (apply forces to bodies, remove expired generators, etc.)
    void update(float deltaTime);

   
	// Queries
    ForceGenerator* findByName(const std::string& name) const;
    std::vector<ForceGenerator*> getAllGenerators() const;
    std::vector<ForceGenerator*> getGeneratorsByType(ForceGeneratorType type) const;
    size_t                       getGeneratorCount() const { return generators.size(); }
    bool                         hasGenerator(const std::string& name) const;

    void printStats() const;
};

#endif // FORCE_GENERATOR_REGISTRY_H