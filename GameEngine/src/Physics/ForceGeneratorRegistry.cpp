#include "../include/Physics/ForceGeneratorRegistry.h"
#include "../include/Physics/ForceGenerator.h"
#include <iostream>
#include <algorithm>

ForceGeneratorRegistry* ForceGeneratorRegistry::instance = nullptr;

ForceGeneratorRegistry::~ForceGeneratorRegistry()
{
    clearAll();
}

ForceGeneratorRegistry& ForceGeneratorRegistry::getInstance()
{
    if (!instance)
        instance = new ForceGeneratorRegistry();
    return *instance;
}

void ForceGeneratorRegistry::initialize(btDiscreteDynamicsWorld* world)
{
    dynamicsWorld = world;
    std::cout << "[ForceGeneratorRegistry] Initialized" << std::endl;
}



// add a generator to the registry, returning a raw pointer (registry retains ownership)
ForceGenerator* ForceGeneratorRegistry::addGenerator(std::unique_ptr<ForceGenerator> generator)
{
    if (!generator)
    {
        std::cerr << "[ForceGeneratorRegistry] Error: Cannot add null generator" << std::endl;
        return nullptr;
    }

    ForceGenerator* ptr = generator.get();
    generators.push_back(std::move(generator));

    std::cout << "[ForceGeneratorRegistry] Added '" << ptr->getName()
        << "' (total: " << generators.size() << ")" << std::endl;

    return ptr;
}

void ForceGeneratorRegistry::removeGenerator(ForceGenerator* generator)
{
    if (!generator) return;

    auto it = std::find_if(generators.begin(), generators.end(),
        [generator](const std::unique_ptr<ForceGenerator>& g) {
            return g.get() == generator;
        });

    if (it != generators.end())
    {
        std::cout << "[ForceGeneratorRegistry] Removed '" << (*it)->getName() << "'" << std::endl;
        generators.erase(it);
    }
}

bool ForceGeneratorRegistry::removeGenerator(const std::string& name)
{
    ForceGenerator* gen = findByName(name);
    if (gen)
    {
        removeGenerator(gen);
        return true;
    }
    return false;
}

void ForceGeneratorRegistry::clearAll()
{
    generators.clear();
    std::cout << "[ForceGeneratorRegistry] All generators cleared" << std::endl;
}


// factory methods for creating common generator types
ForceGenerator* ForceGeneratorRegistry::createWind(const std::string& name,
    const glm::vec3& position,
    float radius,
    const glm::vec3& direction,
    float strength)
{
    return addGenerator(std::make_unique<WindGenerator>(name, position, radius, direction, strength));
}

ForceGenerator* ForceGeneratorRegistry::createGravityWell(const std::string& name,
    const glm::vec3& position,
    float radius,
    float strength)
{
    return addGenerator(std::make_unique<GravityWellGenerator>(name, position, radius, strength));
}

ForceGenerator* ForceGeneratorRegistry::createVortex(const std::string& name,
    const glm::vec3& position,
    float radius,
    const glm::vec3& axis,
    float rotationStrength,
    float pullStrength)
{
    return addGenerator(std::make_unique<VortexGenerator>(
        name, position, radius, axis, rotationStrength, pullStrength));
}

ForceGenerator* ForceGeneratorRegistry::createExplosion(const std::string& name,
    const glm::vec3& position,
    float radius,
    float strength)
{
    return addGenerator(std::make_unique<ExplosionGenerator>(name, position, radius, strength));
}


// update method to apply all active generators to all rigid bodies in the world, then remove expired generators
void ForceGeneratorRegistry::update(float deltaTime)
{
    if (!dynamicsWorld) return;

    // Iterate every collision object in the physics world
    int numObjects = dynamicsWorld->getNumCollisionObjects();
    const btCollisionObjectArray& objArray = dynamicsWorld->getCollisionObjectArray();

    for (auto& gen : generators)
    {
        if (!gen || !gen->isEnabled()) continue;

        for (int i = 0; i < numObjects; ++i)
        {
            btCollisionObject* colObj = objArray[i];
            btRigidBody* body = btRigidBody::upcast(colObj);

            // Skip non-rigid-bodies, sleeping static bodies
            if (!body) continue;
            if (body->isStaticObject()) continue;

            // Get body world position
            const btVector3& btPos = body->getCenterOfMassPosition();
            glm::vec3 bodyPos(btPos.x(), btPos.y(), btPos.z());

            gen->apply(body, bodyPos, deltaTime);
        }
    }

    // Mark explosions as fired after all bodies have been processed this frame,
    // then remove any expired generators.
    for (auto& gen : generators)
    {
        if (gen && gen->getType() == ForceGeneratorType::EXPLOSION && !gen->isExpired())
        {
            static_cast<ExplosionGenerator*>(gen.get())->markFired();
        }
    }

    // Remove expired generators
    generators.erase(
        std::remove_if(generators.begin(), generators.end(),
            [](const std::unique_ptr<ForceGenerator>& g) {
                return g && g->isExpired();
            }),
        generators.end()
    );
}


// querty methods to find generators by name, type, etc.
ForceGenerator* ForceGeneratorRegistry::findByName(const std::string& name) const
{
    for (const auto& gen : generators)
    {
        if (gen->getName() == name)
            return gen.get();
    }
    return nullptr;
}
// get raw pointers to all generators (registry retains ownership)
std::vector<ForceGenerator*> ForceGeneratorRegistry::getAllGenerators() const
{
    std::vector<ForceGenerator*> result;
    result.reserve(generators.size());
    for (const auto& gen : generators)
        result.push_back(gen.get());
    return result;
}

std::vector<ForceGenerator*> ForceGeneratorRegistry::getGeneratorsByType(ForceGeneratorType type) const
{
    std::vector<ForceGenerator*> result;
    for (const auto& gen : generators)
    {
        if (gen->getType() == type)
            result.push_back(gen.get());
    }
    return result;
}

bool ForceGeneratorRegistry::hasGenerator(const std::string& name) const
{
    return findByName(name) != nullptr;
}