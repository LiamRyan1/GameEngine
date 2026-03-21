#include "../include/Rendering/PointLightRegistry.h"
#include <algorithm>
#include <iostream>

uint64_t PointLight::nextID = 1;
PointLightRegistry* PointLightRegistry::instance = nullptr;

PointLightRegistry& PointLightRegistry::getInstance() {
    if (!instance)
        instance = new PointLightRegistry();
    return *instance;
}

PointLight* PointLightRegistry::addLight(const std::string& name,
    const glm::vec3& position,
    const glm::vec3& colour,
    float intensity,
    float radius)
{
    auto light = std::make_unique<PointLight>(name, position, colour, intensity, radius);
    PointLight* raw = light.get();
    lights.push_back(std::move(light));
    std::cout << "[PointLightRegistry] Added '" << name
        << "' (total: " << lights.size() << ")" << std::endl;
    return raw;
}

void PointLightRegistry::removeLight(PointLight* light) {
    if (!light) return;
    auto it = std::find_if(lights.begin(), lights.end(),
        [light](const std::unique_ptr<PointLight>& l) { return l.get() == light; });
    if (it != lights.end()) {
        lights.erase(it);
        std::cout << "[PointLightRegistry] Removed light" << std::endl;
    }
}

bool PointLightRegistry::removeLight(const std::string& name) {
    PointLight* l = findByName(name);
    if (l) { removeLight(l); return true; }
    return false;
}

void PointLightRegistry::clearAll() {
    lights.clear();
    std::cout << "[PointLightRegistry] All lights cleared" << std::endl;
}

PointLight* PointLightRegistry::findByName(const std::string& name) const {
    for (const auto& l : lights)
        if (l->getName() == name) return l.get();
    return nullptr;
}

std::vector<PointLight*> PointLightRegistry::getAllLights() const {
    std::vector<PointLight*> result;
    result.reserve(lights.size());
    for (const auto& l : lights)
        result.push_back(l.get());
    return result;
}