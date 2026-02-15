#include "../include/Physics/ConstraintTemplate.h"
#include "../include/Physics/ConstraintPreset.h"
#include "../include/Scene/GameObject.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

// Initialize static instance
ConstraintTemplateRegistry* ConstraintTemplateRegistry::instance = nullptr;

// ========== ConstraintTemplate ==========

ConstraintTemplate::ConstraintTemplate(const std::string& templateName, ConstraintType templateType)
    : name(templateName), type(templateType)
{
}

// ========== ConstraintTemplateRegistry ==========

ConstraintTemplateRegistry::ConstraintTemplateRegistry()
    : templatesFilePath("constraint_templates.txt")
{
}

ConstraintTemplateRegistry::~ConstraintTemplateRegistry() {
}

ConstraintTemplateRegistry& ConstraintTemplateRegistry::getInstance() {
    if (!instance) {
        instance = new ConstraintTemplateRegistry();
    }
    return *instance;
}

// ========== Template Management ==========

void ConstraintTemplateRegistry::addTemplate(const ConstraintTemplate& templ) {
    // Check if template with same name exists
    for (auto& existing : templates) {
        if (existing.name == templ.name) {
            std::cout << "Updating existing template: " << templ.name << std::endl;
            existing = templ;
            return;
        }
    }

    templates.push_back(templ);
    std::cout << "Added new template: " << templ.name << std::endl;
}

bool ConstraintTemplateRegistry::removeTemplate(const std::string& name) {
    auto it = std::find_if(templates.begin(), templates.end(),
        [&name](const ConstraintTemplate& t) { return t.name == name; });

    if (it != templates.end()) {
        templates.erase(it);
        std::cout << "Removed template: " << name << std::endl;
        return true;
    }

    return false;
}

void ConstraintTemplateRegistry::clearAllTemplates() {
    templates.clear();
    std::cout << "Cleared all constraint templates" << std::endl;
}

// ========== Queries ==========

const ConstraintTemplate* ConstraintTemplateRegistry::getTemplate(const std::string& name) const {
    for (const auto& templ : templates) {
        if (templ.name == name) {
            return &templ;
        }
    }
    return nullptr;
}

std::vector<std::string> ConstraintTemplateRegistry::getTemplateNames() const {
    std::vector<std::string> names;
    names.reserve(templates.size());

    for (const auto& templ : templates) {
        names.push_back(templ.name);
    }

    return names;
}

bool ConstraintTemplateRegistry::hasTemplate(const std::string& name) const {
    return getTemplate(name) != nullptr;
}

// ========== Smart Pivot Calculation ==========

glm::vec3 ConstraintTemplateRegistry::calculateSmartPivot(GameObject* obj, const glm::vec3& relativeOffset, bool useEdge) {
    if (!obj) return glm::vec3(0.0f);

    glm::vec3 scale = obj->getScale();

    // For edge attachment (like hinges), use half-extents
    if (useEdge) {
        return glm::vec3(
            relativeOffset.x * scale.x * 0.5f,
            relativeOffset.y * scale.y * 0.5f,
            relativeOffset.z * scale.z * 0.5f
        );
    }

    // For center attachment, just use the offset directly
    return relativeOffset;
}

glm::vec3 ConstraintTemplateRegistry::calculateHingePivot(GameObject* objA, GameObject* objB, const glm::vec3& axis) {
    if (!objA) return glm::vec3(0.0f);

    glm::vec3 scale = objA->getScale();

    // Determine which axis is the hinge axis and place pivot at edge
    glm::vec3 pivot(0.0f);

    // Find dominant axis
    float maxComponent = 0.0f;
    int dominantAxis = 0;
    for (int i = 0; i < 3; ++i) {
        if (std::abs(axis[i]) > maxComponent) {
            maxComponent = std::abs(axis[i]);
            dominantAxis = i;
        }
    }

    // Place pivot at edge perpendicular to hinge axis
    // For a vertical hinge (Y axis), place at X edge
    // For a horizontal hinge (X or Z), place at appropriate edge
    if (dominantAxis == 1) { // Y-axis hinge
        pivot.x = -scale.x * 0.5f; // Left edge
    }
    else if (dominantAxis == 0) { // X-axis hinge
        pivot.z = -scale.z * 0.5f; // Front edge
    }
    else { // Z-axis hinge
        pivot.x = -scale.x * 0.5f; // Left edge
    }

    return pivot;
}

// ========== Apply Template ==========

std::unique_ptr<Constraint> ConstraintTemplateRegistry::applyTemplate(
    const std::string& templateName,
    GameObject* objA,
    GameObject* objB)
{
    const ConstraintTemplate* templ = getTemplate(templateName);
    if (!templ) {
        std::cerr << "Error: Template '" << templateName << "' not found" << std::endl;
        return nullptr;
    }

    if (!objA || !objA->hasPhysics()) {
        std::cerr << "Error: Cannot apply template - objA missing or no physics" << std::endl;
        return nullptr;
    }

    std::unique_ptr<Constraint> constraint;

    switch (templ->type) {
    case ConstraintType::FIXED:
        constraint = ConstraintPreset::createFixed(objA, objB);
        break;

    case ConstraintType::HINGE: {
        HingeParams params = templ->hingeParams;

        // Recalculate pivots based on object size if using relative positioning
        if (glm::length(params.pivotA) < 0.01f) {
            params.pivotA = calculateHingePivot(objA, objB, params.axisA);
        }

        if (objB && glm::length(params.pivotB) < 0.01f) {
            params.pivotB = calculateHingePivot(objB, objA, params.axisB);
        }

        constraint = ConstraintPreset::createHinge(objA, objB, params);
        break;
    }

    case ConstraintType::SLIDER: {
        SliderParams params = templ->sliderParams;
        constraint = ConstraintPreset::createSlider(objA, objB, params);
        break;
    }

    case ConstraintType::SPRING: {
        SpringParams params = templ->springParams;
        constraint = ConstraintPreset::createSpring(objA, objB, params);
        break;
    }

    case ConstraintType::GENERIC_6DOF: {
        Generic6DofParams params = templ->dofParams;
        constraint = ConstraintPreset::createGeneric6Dof(objA, objB, params);
        break;
    }
    }

    if (constraint) {
        constraint->setName(templ->name);

        if (templ->breakable) {
            constraint->setBreakingThreshold(templ->breakForce, templ->breakTorque);
        }

        std::cout << "Applied template '" << templateName << "' to create constraint" << std::endl;
    }

    return constraint;
}

// ========== File I/O (Simple Text Format) ==========

bool ConstraintTemplateRegistry::saveToFile(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filepath << std::endl;
        return false;
    }

    file << "# Constraint Templates\n";
    file << "# Format: NAME|TYPE|PARAMS...\n\n";
    file << "COUNT=" << templates.size() << "\n\n";

    for (const auto& templ : templates) {
        file << "TEMPLATE_START\n";
        file << "name=" << templ.name << "\n";
        file << "type=" << static_cast<int>(templ.type) << "\n";
        file << "breakable=" << (templ.breakable ? 1 : 0) << "\n";
        file << "breakForce=" << templ.breakForce << "\n";
        file << "breakTorque=" << templ.breakTorque << "\n";
        file << "description=" << templ.description << "\n";

        // Type-specific parameters
        switch (templ.type) {
        case ConstraintType::HINGE: {
            const auto& p = templ.hingeParams;
            file << "hinge_pivotA=" << p.pivotA.x << "," << p.pivotA.y << "," << p.pivotA.z << "\n";
            file << "hinge_pivotB=" << p.pivotB.x << "," << p.pivotB.y << "," << p.pivotB.z << "\n";
            file << "hinge_axisA=" << p.axisA.x << "," << p.axisA.y << "," << p.axisA.z << "\n";
            file << "hinge_axisB=" << p.axisB.x << "," << p.axisB.y << "," << p.axisB.z << "\n";
            file << "hinge_useLimits=" << (p.useLimits ? 1 : 0) << "\n";
            file << "hinge_lowerLimit=" << p.lowerLimit << "\n";
            file << "hinge_upperLimit=" << p.upperLimit << "\n";
            file << "hinge_useMotor=" << (p.useMotor ? 1 : 0) << "\n";
            file << "hinge_motorVelocity=" << p.motorTargetVelocity << "\n";
            file << "hinge_motorImpulse=" << p.motorMaxImpulse << "\n";
            break;
        }
        case ConstraintType::SLIDER: {
            const auto& p = templ.sliderParams;
            file << "slider_framePosA=" << p.frameAPos.x << "," << p.frameAPos.y << "," << p.frameAPos.z << "\n";
            file << "slider_framePosB=" << p.frameBPos.x << "," << p.frameBPos.y << "," << p.frameBPos.z << "\n";
            file << "slider_useLimits=" << (p.useLimits ? 1 : 0) << "\n";
            file << "slider_lowerLimit=" << p.lowerLimit << "\n";
            file << "slider_upperLimit=" << p.upperLimit << "\n";
            file << "slider_useMotor=" << (p.useMotor ? 1 : 0) << "\n";
            file << "slider_motorVelocity=" << p.motorTargetVelocity << "\n";
            file << "slider_motorForce=" << p.motorMaxForce << "\n";
            break;
        }
        case ConstraintType::SPRING: {
            const auto& p = templ.springParams;
            file << "spring_pivotA=" << p.pivotA.x << "," << p.pivotA.y << "," << p.pivotA.z << "\n";
            file << "spring_pivotB=" << p.pivotB.x << "," << p.pivotB.y << "," << p.pivotB.z << "\n";
            for (int i = 0; i < 6; ++i) {
                file << "spring_enabled" << i << "=" << (p.enableSpring[i] ? 1 : 0) << "\n";
                file << "spring_stiffness" << i << "=" << p.stiffness[i] << "\n";
                file << "spring_damping" << i << "=" << p.damping[i] << "\n";
            }
            break;
        }
        default:
            break;
        }

        file << "TEMPLATE_END\n\n";
    }

    file.close();
    std::cout << "Saved " << templates.size() << " templates to " << filepath << std::endl;
    return true;
}

bool ConstraintTemplateRegistry::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "Note: No template file found at " << filepath << " (this is normal for first run)" << std::endl;
        return false;
    }

    templates.clear();

    std::string line;
    ConstraintTemplate currentTemplate;
    bool inTemplate = false;

    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        if (line == "TEMPLATE_START") {
            inTemplate = true;
            currentTemplate = ConstraintTemplate();
            continue;
        }

        if (line == "TEMPLATE_END") {
            if (inTemplate) {
                templates.push_back(currentTemplate);
                inTemplate = false;
            }
            continue;
        }

        if (!inTemplate) continue;

        // Parse key=value pairs
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        // Parse common fields
        if (key == "name") currentTemplate.name = value;
        else if (key == "type") currentTemplate.type = static_cast<ConstraintType>(std::stoi(value));
        else if (key == "breakable") currentTemplate.breakable = (std::stoi(value) != 0);
        else if (key == "breakForce") currentTemplate.breakForce = std::stof(value);
        else if (key == "breakTorque") currentTemplate.breakTorque = std::stof(value);
        else if (key == "description") currentTemplate.description = value;

        // Parse hinge parameters
        else if (key == "hinge_pivotA") {
            std::istringstream ss(value);
            char comma;
            ss >> currentTemplate.hingeParams.pivotA.x >> comma
                >> currentTemplate.hingeParams.pivotA.y >> comma
                >> currentTemplate.hingeParams.pivotA.z;
        }
        else if (key == "hinge_axisA") {
            std::istringstream ss(value);
            char comma;
            ss >> currentTemplate.hingeParams.axisA.x >> comma
                >> currentTemplate.hingeParams.axisA.y >> comma
                >> currentTemplate.hingeParams.axisA.z;
        }
        else if (key == "hinge_useLimits") currentTemplate.hingeParams.useLimits = (std::stoi(value) != 0);
        else if (key == "hinge_lowerLimit") currentTemplate.hingeParams.lowerLimit = std::stof(value);
        else if (key == "hinge_upperLimit") currentTemplate.hingeParams.upperLimit = std::stof(value);
        else if (key == "hinge_useMotor") currentTemplate.hingeParams.useMotor = (std::stoi(value) != 0);
        else if (key == "hinge_motorVelocity") currentTemplate.hingeParams.motorTargetVelocity = std::stof(value);
        else if (key == "hinge_motorImpulse") currentTemplate.hingeParams.motorMaxImpulse = std::stof(value);

        // Parse slider parameters
        else if (key == "slider_useLimits") currentTemplate.sliderParams.useLimits = (std::stoi(value) != 0);
        else if (key == "slider_lowerLimit") currentTemplate.sliderParams.lowerLimit = std::stof(value);
        else if (key == "slider_upperLimit") currentTemplate.sliderParams.upperLimit = std::stof(value);

        // Parse spring parameters
        else if (key.find("spring_enabled") == 0) {
            int axis = std::stoi(key.substr(14)); // "spring_enabled"
            currentTemplate.springParams.enableSpring[axis] = (std::stoi(value) != 0);
        }
        else if (key.find("spring_stiffness") == 0) {
            int axis = std::stoi(key.substr(16));
            currentTemplate.springParams.stiffness[axis] = std::stof(value);
        }
        else if (key.find("spring_damping") == 0) {
            int axis = std::stoi(key.substr(14));
            currentTemplate.springParams.damping[axis] = std::stof(value);
        }
    }

    file.close();
    std::cout << "Loaded " << templates.size() << " templates from " << filepath << std::endl;
    return true;
}

bool ConstraintTemplateRegistry::save() {
    return saveToFile(templatesFilePath);
}

bool ConstraintTemplateRegistry::load() {
    return loadFromFile(templatesFilePath);
}

// ========== Default Templates ==========

void ConstraintTemplateRegistry::initializeDefaults() {
 }

// ========== Debug ==========

void ConstraintTemplateRegistry::printTemplates() const {
    std::cout << "\n=== Constraint Templates ===" << std::endl;
    std::cout << "Total templates: " << templates.size() << std::endl;

    for (const auto& templ : templates) {
        std::cout << "\n- " << templ.name << " (" << static_cast<int>(templ.type) << ")" << std::endl;
        if (!templ.description.empty()) {
            std::cout << "  Description: " << templ.description << std::endl;
        }
        if (templ.breakable) {
            std::cout << "  Breakable: force=" << templ.breakForce
                << ", torque=" << templ.breakTorque << std::endl;
        }
    }

    std::cout << "===========================\n" << std::endl;
}