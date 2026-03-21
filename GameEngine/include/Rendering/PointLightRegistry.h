#ifndef POINT_LIGHT_REGISTRY_H
#define POINT_LIGHT_REGISTRY_H

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "PointLight.h"

class PointLightRegistry {
private:
    static PointLightRegistry* instance;
    std::vector<std::unique_ptr<PointLight>> lights;
    PointLightRegistry() = default;

public:
    ~PointLightRegistry() = default;
    static PointLightRegistry& getInstance();

    PointLight* addLight(const std::string& name,
        const glm::vec3& position,
        const glm::vec3& colour = glm::vec3(1.0f),
        float intensity = 1.0f,
        float radius = 10.0f);

    void removeLight(PointLight* light);
    bool removeLight(const std::string& name);
    void clearAll();

    PointLight* findByName(const std::string& name) const;
    std::vector<PointLight*> getAllLights() const;
    size_t getLightCount() const { return lights.size(); }
};

#endif // POINT_LIGHT_REGISTRY_H