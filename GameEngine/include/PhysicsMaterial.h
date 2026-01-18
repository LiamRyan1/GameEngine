#ifndef PHYSICS_MATERIAL_H
#define PHYSICS_MATERIAL_H

#include <string>
#include <unordered_map>
#include <memory>

// physical material properties for rigid bodies
// Wraps Bullet Physics friction and restitution (bounciness) values
class PhysicsMaterial {
public:
    // Material properties
    float friction;      // Surface friction (0.0 = low grip, 1.0+ = high grip)
    float restitution;   // Bounciness (0.0 = no bounce, 1.0 = perfect bounce-no energy loss)
  
    std::string name;    // Material id

	// Materials properties constructor
    PhysicsMaterial(const std::string& name,
        float friction,
        float restitution);

    // Default constructor (creates "Default" material)
    PhysicsMaterial();

    // common material types
    static PhysicsMaterial Default();
    static PhysicsMaterial Wood();
    static PhysicsMaterial Metal();
    static PhysicsMaterial Rubber();
    static PhysicsMaterial Ice();
    static PhysicsMaterial Concrete();
    static PhysicsMaterial Plastic();
    static PhysicsMaterial Glass();
};

// Global material registry - manages and provides access to materials
class MaterialRegistry {
private:
    MaterialRegistry();  // Private constructor for singleton
    std::unordered_map<std::string, PhysicsMaterial> materials;

    // Prevent copying
    MaterialRegistry(const MaterialRegistry&) = delete;
    MaterialRegistry& operator=(const MaterialRegistry&) = delete;

public:
    // Get the singleton instance
    static MaterialRegistry& getInstance();

    // Register a new material or update existing one
    void registerMaterial(const PhysicsMaterial& material);

    // Get material by name (returns Default if not found)
    const PhysicsMaterial& getMaterial(const std::string& name) const;

    // Check if material exists
    bool hasMaterial(const std::string& name) const;

    // Initialize with common presets
    void initializeDefaults();
};


#endif // PHYSICS_MATERIAL_H