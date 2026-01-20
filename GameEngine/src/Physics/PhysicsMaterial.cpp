#include "../include/PhysicsMaterial.h"
#include <iostream>


PhysicsMaterial::PhysicsMaterial(const std::string& name,
    float friction,
    float restitution)
    : name(name), friction(friction), restitution(restitution)
{
}
// Default material constructor
PhysicsMaterial::PhysicsMaterial()
    : PhysicsMaterial("Default", 0.5f, 0.3f) {
}



PhysicsMaterial PhysicsMaterial::Default() {
    return PhysicsMaterial("Default", 0.5f, 0.3f);
}

PhysicsMaterial PhysicsMaterial::Wood() {
    // Wood: moderate friction, some bounce
    return PhysicsMaterial("Wood", 0.8f, 0.4f);
}

PhysicsMaterial PhysicsMaterial::Metal() {
    // Metal: low friction, high bounce
    return PhysicsMaterial("Metal", 0.3f, 0.7f);
}

PhysicsMaterial PhysicsMaterial::Rubber() {
    // Rubber: MAXIMUM BOUNCE - should bounce almost as high as it fell
    return PhysicsMaterial("Rubber", 1.0f, 0.95f);
}

PhysicsMaterial PhysicsMaterial::Ice() {
    // Ice: ZERO friction, almost no bounce - should slide forever
    return PhysicsMaterial("Ice", 0.0f, 0.05f);
}

PhysicsMaterial PhysicsMaterial::Concrete() {
    // Concrete: high friction, ZERO bounce - should stop dead
    return PhysicsMaterial("Concrete", 1.5f, 0.0f);
}

PhysicsMaterial PhysicsMaterial::Plastic() {
    // Plastic: medium everything
    return PhysicsMaterial("Plastic", 0.5f, 0.5f);
}

PhysicsMaterial PhysicsMaterial::Glass() {
    // Glass: low friction, low bounce
    return PhysicsMaterial("Glass", 0.2f, 0.2f);
}

// register new materials and manage existing ones
MaterialRegistry& MaterialRegistry::getInstance() {
	
    static MaterialRegistry instance;
    return instance;
}


MaterialRegistry::MaterialRegistry() {
    initializeDefaults();
}
// Register or update a material in the map - replaces if name already exists
void MaterialRegistry::registerMaterial(const PhysicsMaterial& material) {
    materials[material.name] = material;
    std::cout << "Registered material: " << material.name
        << " (friction=" << material.friction
        << ", restitution=" << material.restitution
         << ")" << std::endl;
}
// Retrieve material by name, or return Default if not found
const PhysicsMaterial& MaterialRegistry::getMaterial(const std::string& name) const {
    auto it = materials.find(name);
    if (it != materials.end()) {
        return it->second;
    }

    // Return default material if not found
    std::cerr << "Warning: Material '" << name << "' not found, using Default" << std::endl;
    return materials.at("Default");
}
// Check if material exists in the registry
bool MaterialRegistry::hasMaterial(const std::string& name) const {
    return materials.find(name) != materials.end();
}

// Get all material names for UI dropdowns
std::vector<std::string> MaterialRegistry::getAllMaterialNames() const {
    std::vector<std::string> names;
    names.reserve(materials.size());

    for (const auto& pair : materials) {
        names.push_back(pair.first);
    }

    return names;
}

void MaterialRegistry::initializeDefaults() {
    std::cout << "Initializing default physics materials..." << std::endl;

    registerMaterial(PhysicsMaterial::Default());
    registerMaterial(PhysicsMaterial::Wood());
    registerMaterial(PhysicsMaterial::Metal());
    registerMaterial(PhysicsMaterial::Rubber());
    registerMaterial(PhysicsMaterial::Ice());
    registerMaterial(PhysicsMaterial::Concrete());
    registerMaterial(PhysicsMaterial::Plastic());
    registerMaterial(PhysicsMaterial::Glass());

    std::cout << "Physics materials initialized (" << materials.size() << " materials)" << std::endl;
}