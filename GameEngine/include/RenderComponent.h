#ifndef RENDERCOMPONENT_H
#define RENDERCOMPONENT_H

#include "Component.h"
#include "Mesh.h"
#include <string>

/**
 * @brief Defines visual shape types for rendering.
 */
enum class ShapeType {
    CUBE,
    SPHERE,
    CAPSULE,
};

/**
 * @brief Component that stores rendering-specific data.
 *
 * This component holds:
 * - Shape type (cube, sphere, capsule)
 * - Texture path
 * - Material properties (future: color, shininess, etc.)
 *
 * The Renderer reads this data to draw the object.
 */
class RenderComponent : public Component {
private:
    ShapeType shapeType;
    std::string texturePath;
	Mesh* renderMesh; // Pointer to the mesh used for rendering

public:
    RenderComponent(ShapeType type, const std::string& texture = "")
        : shapeType(type), texturePath(texture) {
    }

    ShapeType getShapeType() const { return shapeType; }
    const std::string& getTexturePath() const { return texturePath; }

    void setShapeType(ShapeType type) { shapeType = type; }
    void setTexturePath(const std::string& path) { texturePath = path; }
    
    void setRenderMesh(Mesh* mesh) { renderMesh = mesh; }
    Mesh* getRenderMesh() const { return renderMesh; }

};

#endif // RENDERCOMPONENT_H