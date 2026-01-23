#pragma once

// Gizmo goal:
// - Draw editor gizmos (translate/rotate/scale) using ImGui overlay
// - No renderer changes 
// - Handle picking + dragging in editor mode only
//
// This file declares an EditorGizmo helper that:
// 1) Draws axis lines on screen for a selected object
// 2) Detects mouse hover/click on an axis
// 3) When dragging, moves the object along that world axis

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera;
class GameObject;
struct GLFWwindow;

class EditorGizmo
{
public:
    // Start with Translate
    enum class Mode { Translate /*, Rotate, Scale */ };

    // Which axis is hovered/active
    enum class Axis { None, X, Y, Z };

    EditorGizmo();

    void setMode(Mode m) { mode = m; }
    Mode getMode() const { return mode; }

    // update():
    // - Runs input + math logic:
    //   - hover detection
    //   - mouse click -> begin drag
    //   - mouse move -> update object position
    //   - mouse release -> stop drag
    //
    // Returns true if the gizmo is currently using the mouse (dragging)
    // so the engine can disable editor picking during drag.
    bool update(
        GLFWwindow* window,
        int fbW, int fbH,
        const Camera& camera,
        GameObject* selectedObject,
        bool editorMode,
        bool uiWantsMouse);

    // draw():
    // - Renders the axis lines as a 2D overlay via ImGui
    // - Call after update(), before ImGui::Render()
    void draw(int fbW, int fbH, const Camera& camera, GameObject* selectedObject);

    // Expose dragging state
    bool isDragging() const { return dragging; }

private:
    // Current gizmo mode
    Mode mode = Mode::Translate;

    // Hot axis is what the mouse is hovering this frame
    Axis hotAxis = Axis::None;

    // Active axis is the axis clicked on and are dragging
    Axis activeAxis = Axis::None;

    // True while dragging along an axis
    bool dragging = false;

    // Store the object's position at the moment drag begins
    // Spply relative movement cleanly.
    glm::vec3 dragStartObjPos{ 0.0f };

    // Store where the mouse-ray hit the drag plane at drag start
    // Compute mouse delta in world space.
    glm::vec3 dragStartHitPoint{ 0.0f };

    // Tuning values
    float axisPickThresholdPx = 10.0f; // how close mouse must be to an axis line (in pixels)
    float axisLineThickness = 3.0f;  // line thickness for axis draw

private:
    // Converts a world space position -> screen pixel position
    // Returns false if point is behind the camera / invalid.
    static bool worldToScreen(
        const glm::vec3& world,
        const glm::mat4& view,
        const glm::mat4& proj,
        int fbW, int fbH,
        glm::vec2& outScreen);

    // Distance from a point to a line segment in 2D.
    // Used for "hover detection" on the axis line.
    static float distancePointToSegment2D(
        const glm::vec2& p,
        const glm::vec2& a,
        const glm::vec2& b);

    // Builds a ray from the mouse position through the camera:
    // rayOrigin = camera position
    // rayDir    = direction in world space (normalized)
    static bool buildMouseRay(
        GLFWwindow* window,
        int fbW, int fbH,
        const Camera& camera,
        glm::vec3& outOrigin,
        glm::vec3& outDir);

    // Intersects a ray with a plane.
    // Plane is defined by: point on plane + plane normal.
    static bool rayPlaneIntersection(
        const glm::vec3& ro,
        const glm::vec3& rd,
        const glm::vec3& planePoint,
        const glm::vec3& planeNormal,
        glm::vec3& outHit);

    // Helper: returns unit direction for the chosen axis
    static glm::vec3 axisDir(Axis a);
};

