#include "../include/Physics/PhysicsMaterial.h"
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

/**
 * @brief Gets the singleton MaterialRegistry instance.
 *
 * Implements Meyer's Singleton pattern - thread-safe in C++11 and later.
 * The instance is created on first access and persists for the program lifetime.
 *
 * @return Reference to the global MaterialRegistry
 */
MaterialRegistry& MaterialRegistry::getInstance() {
	
    static MaterialRegistry instance;
    return instance;
}

/**
 * @brief Private constructor - initializes registry with default materials.
 *
 * Called automatically on first getInstance() access.
 * Registers all preset materials (Default, Wood, Metal, etc.).
 */
MaterialRegistry::MaterialRegistry() {
    initializeDefaults();
}

/**
 * @brief Registers or updates a material in the registry.
 *
 * If a material with the same name exists, it will be replaced.
 * This allows runtime customization of material properties.
 *
 * Use cases:
 * - Adding custom materials from game configuration
 * - Editor tools for tweaking physics properties
 * - Mod support for user-defined materials
 *
 * @param material The material to register (name must be unique)
 *
 * @note Logs registration to console for debugging
 *
 * @example
 * PhysicsMaterial custom("Bouncy", 0.8f, 0.9f);
 * MaterialRegistry::getInstance().registerMaterial(custom);
 */
void MaterialRegistry::registerMaterial(const PhysicsMaterial& material) {
    materials[material.name] = material;
    std::cout << "Registered material: " << material.name
        << " (friction=" << material.friction
        << ", restitution=" << material.restitution
         << ")" << std::endl;
}


/**
 * @brief Retrieves a material by name.
 *
 * Looks up a material in the registry. If not found, returns the "Default"
 * material as a safe fallback and logs a warning.
 *
 * @param name The material name to look up (case-sensitive)
 * @return Const reference to the material (or Default if not found)
 *
 * @warning If "Default" material is missing (should never happen), this will throw.
 * @note Logs a warning to stderr if material not found
 *
 * @example
 * const PhysicsMaterial& wood = MaterialRegistry::getInstance().getMaterial("Wood");
 * rigidBody->setFriction(wood.friction);
 */
const PhysicsMaterial& MaterialRegistry::getMaterial(const std::string& name) const {
    auto it = materials.find(name);
    if (it != materials.end()) {
        return it->second;
    }

    // Return default material if not found
    std::cerr << "Warning: Material '" << name << "' not found, using Default" << std::endl;
    return materials.at("Default");
}


/**
 * @brief Checks if a material exists in the registry.
 *
 * Use this to validate material names before attempting to use them,
 * especially when loading from user data or configuration files.
 *
 * @param name Material name to check
 * @return true if material exists, false otherwise
 *
 * @example
 * if (registry.hasMaterial("CustomWood")) {
 *     // Safe to use
 * }
 */
bool MaterialRegistry::hasMaterial(const std::string& name) const {
    return materials.find(name) != materials.end();
}

/**
 * @brief Gets all registered material names.
 *
 * Returns a list of all material names currently in the registry.
 * Useful for populating UI dropdowns or displaying available options.
 *
 * @return Vector of material name strings
 *
 * @note Order is unspecified (comes from unordered_map iteration)
 * @note Vector is returned by value (copy) but is small enough to be efficient
 *
 * @example
 * // Populate ImGui combo box
 * std::vector<std::string> names = registry.getAllMaterialNames();
 * for (const auto& name : names) {
 *     ImGui::Selectable(name.c_str());
 * }
 */
std::vector<std::string> MaterialRegistry::getAllMaterialNames() const {
    std::vector<std::string> names;
    names.reserve(materials.size());

    for (const auto& pair : materials) {
        names.push_back(pair.first);
    }

    return names;
}


/**
 * @brief Initializes the registry with all preset materials.
 *
 * Called automatically by the constructor. Registers the 8 built-in
 * material presets: Default, Wood, Metal, Rubber, Ice, Concrete, Plastic, Glass.
 *
 * @note This method is idempotent - safe to call multiple times if needed
 * @note Logs initialization progress to console
 */
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