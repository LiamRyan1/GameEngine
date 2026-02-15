#ifndef CONSTRAINTTEMPLATE_H
#define CONSTRAINTTEMPLATE_H

#include "ConstraintParams.h"
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

class GameObject;
class Constraint;

/**
 * @brief A saved constraint configuration that can be reused.
 *
 * Templates store all constraint parameters and can be applied to different
 * object pairs. Pivots and frames are stored as relative offsets that get
 * recalculated based on object size when applied.
 */
struct ConstraintTemplate {
    std::string name;
    ConstraintType type;

    // Type-specific parameters
    HingeParams hingeParams;
    SliderParams sliderParams;
    SpringParams springParams;
    Generic6DofParams dofParams;

    // Breaking settings
    bool breakable = false;
    float breakForce = 1000.0f;
    float breakTorque = 1000.0f;

    // Metadata
    std::string description;

    ConstraintTemplate() = default;
    ConstraintTemplate(const std::string& templateName, ConstraintType templateType);
};

/**
 * @brief Manages constraint templates - saving, loading, and applying them.
 *
 * Singleton registry that stores user-created constraint templates and provides
 * smart pivot calculation when applying templates to objects with different sizes.
 */
class ConstraintTemplateRegistry {
private:
    static ConstraintTemplateRegistry* instance;

    std::vector<ConstraintTemplate> templates;
    std::string templatesFilePath;

    ConstraintTemplateRegistry();

    // Smart pivot calculation based on object bounds
    glm::vec3 calculateSmartPivot(GameObject* obj, const glm::vec3& relativeOffset, bool useEdge = false);
    glm::vec3 calculateHingePivot(GameObject* objA, GameObject* objB, const glm::vec3& axis);

public:
    ~ConstraintTemplateRegistry();

    ConstraintTemplateRegistry(const ConstraintTemplateRegistry&) = delete;
    ConstraintTemplateRegistry& operator=(const ConstraintTemplateRegistry&) = delete;

    static ConstraintTemplateRegistry& getInstance();

    // Template management
    void addTemplate(const ConstraintTemplate& templ);
    bool removeTemplate(const std::string& name);
    void clearAllTemplates();

    // Queries
    const ConstraintTemplate* getTemplate(const std::string& name) const;
    std::vector<std::string> getTemplateNames() const;
    const std::vector<ConstraintTemplate>& getAllTemplates() const { return templates; }
    bool hasTemplate(const std::string& name) const;
    int getTemplateCount() const { return templates.size(); }

    // Apply template to create constraint
    std::unique_ptr<Constraint> applyTemplate(
        const std::string& templateName,
        GameObject* objA,
        GameObject* objB = nullptr
    );

    // File I/O
    bool saveToFile(const std::string& filepath);
    bool loadFromFile(const std::string& filepath);

    // Default file operations
    bool save(); // Save to default location
    bool load(); // Load from default location

    // Initialize with a few useful starter templates
    void initializeDefaults();

    // Debug
    void printTemplates() const;
};

#endif // CONSTRAINTTEMPLATE_H