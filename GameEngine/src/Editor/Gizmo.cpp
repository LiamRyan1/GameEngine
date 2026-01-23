#include "../include/Editor/Gizmo.h"

#include <GLFW/glfw3.h>
#include "imgui.h"

#include "../include/Camera.h"
#include "../include/GameObject.h"

#include <glm/gtc/matrix_transform.hpp>

// Constructor
EditorGizmo::EditorGizmo() {}

// Returns the unit direction for each world axis
glm::vec3 EditorGizmo::axisDir(Axis a)
{
    switch (a)
    {
    case Axis::X: return glm::vec3(1, 0, 0);
    case Axis::Y: return glm::vec3(0, 1, 0);
    case Axis::Z: return glm::vec3(0, 0, 1);
    default:      return glm::vec3(0, 0, 0);
    }
}

// worldToScreen():
// Converts world position -> clip space -> NDC -> screen pixels
// Clip = proj * view * world
// NDC = clip.xyz / clip.w
// screen.x = (ndc.x * 0.5 + 0.5) * width
// screen.y = (1 - (ndc.y * 0.5 + 0.5)) * height
// Returns false if clip.w <= 0 (behind camera).
bool EditorGizmo::worldToScreen(const glm::vec3& world,
    const glm::mat4& view,
    const glm::mat4& proj,
    int fbW, int fbH,
    glm::vec2& outScreen)
{
    glm::vec4 clip = proj * view * glm::vec4(world, 1.0f);

    // If w is <= 0, point is behind the camera (not drawable in a stable way)
    if (clip.w <= 0.00001f)
        return false;

    glm::vec3 ndc = glm::vec3(clip) / clip.w;

    // Convert [-1..1] to pixels
    outScreen.x = (ndc.x * 0.5f + 0.5f) * fbW;
    outScreen.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * fbH;
    return true;
}

// Distance from point P to segment AB in 2D.
// Used so that axis lines are clickable with a tolerance in pixels.
float EditorGizmo::distancePointToSegment2D(const glm::vec2& p,
    const glm::vec2& a,
    const glm::vec2& b)
{
    glm::vec2 ab = b - a;
    float abLen2 = glm::dot(ab, ab);

    // If A and B are basically the same point, distance is to A
    if (abLen2 < 0.00001f)
        return glm::length(p - a);

    // Project P onto AB, get parameter t in [0..1]
    float t = glm::dot(p - a, ab) / abLen2;
    t = glm::clamp(t, 0.0f, 1.0f);

    glm::vec2 closest = a + t * ab;
    return glm::length(p - closest);
}

// buildMouseRay():
// Creates the same ray already done for picking:
// mouse pixels -> NDC -> eye -> world direction
// outOrigin = camera position
// outDir    = normalized world direction
bool EditorGizmo::buildMouseRay(GLFWwindow* window,
    int fbW, int fbH,
    const Camera& camera,
    glm::vec3& outOrigin,
    glm::vec3& outDir)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Screen -> NDC
    float x = (2.0f * static_cast<float>(mouseX)) / fbW - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / fbH;

    // Mouse point in clip space on near plane (z = -1)
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    // Unproject to eye space
    glm::mat4 projection = camera.getProjectionMatrix(static_cast<float>(fbW) / fbH);
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;

    // Convert from point to direction:
    // z = -1 (forward), w = 0 (direction)
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Eye -> World
    glm::mat4 invView = glm::inverse(camera.getViewMatrix());
    outDir = glm::normalize(glm::vec3(invView * rayEye));
    outOrigin = camera.getPosition();

    return true;
}

// rayPlaneIntersection():
// Solve ray-plane intersection:
// plane: dot((P - planePoint), planeNormal) = 0
// ray:   P = ro + t * rd
//
// Substitute and solve for t:
// t = dot((planePoint - ro), planeNormal) / dot(rd, planeNormal)
//
// If denom is 0 => ray parallel to plane.
bool EditorGizmo::rayPlaneIntersection(const glm::vec3& ro,
    const glm::vec3& rd,
    const glm::vec3& planePoint,
    const glm::vec3& planeNormal,
    glm::vec3& outHit)
{
    float denom = glm::dot(rd, planeNormal);
    if (std::abs(denom) < 0.00001f)
        return false;

    float t = glm::dot(planePoint - ro, planeNormal) / denom;

    // negative t means intersection behind ray origin
    if (t < 0.0f)
        return false;

    outHit = ro + rd * t;
    return true;
}

// update():
// Handles:
// - which axis is hovered
// - click on axis starts drag
// - while dragging: compute new hit point on drag plane,
//   project delta onto axis, and set object position.
bool EditorGizmo::update(GLFWwindow* window,
    int fbW, int fbH,
    const Camera& camera,
    GameObject* selectedObject,
    bool editorMode,
    bool uiWantsMouse)
{
    // Reset hovered axis each frame
    hotAxis = Axis::None;

    // not in editor mode or no selection exists,
    // stop any dragging and do nothing.
    if (!editorMode || !selectedObject)
    {
        dragging = false;
        activeAxis = Axis::None;
        return false;
    }

    // Prepare matrices (for projection to screen space)
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 proj = camera.getProjectionMatrix(static_cast<float>(fbW) / fbH);

    // Gizmo origin is the selected object's position
    glm::vec3 originW = selectedObject->getPosition();

    // Project origin to screen
    glm::vec2 originS;
    if (!worldToScreen(originW, view, proj, fbW, fbH, originS))
        return false;

    // Choose a world-space length for axes.
    // Scale it based on distance to camera so the gizmo
    // stays about the same size on screen.
    float distToCam = glm::length(camera.getPosition() - originW);
    float axisLenWorld = glm::clamp(distToCam * 0.15f, 0.5f, 6.0f);

    // Compute world endpoints for each axis
    glm::vec3 xEndW = originW + glm::vec3(1, 0, 0) * axisLenWorld;
    glm::vec3 yEndW = originW + glm::vec3(0, 1, 0) * axisLenWorld;
    glm::vec3 zEndW = originW + glm::vec3(0, 0, 1) * axisLenWorld;

    // Project endpoints to screen
    glm::vec2 xEndS, yEndS, zEndS;
    bool okX = worldToScreen(xEndW, view, proj, fbW, fbH, xEndS);
    bool okY = worldToScreen(yEndW, view, proj, fbW, fbH, yEndS);
    bool okZ = worldToScreen(zEndW, view, proj, fbW, fbH, zEndS);

    // Current mouse position in screen pixels
    ImVec2 mouse = ImGui::GetMousePos();
    glm::vec2 mouseS(mouse.x, mouse.y);

    // Hover test:
    // Compute distance from mouse to each axis line segment on screen.
    // Pick the closest axis within threshold.
    float best = axisPickThresholdPx;

    if (okX)
    {
        float d = distancePointToSegment2D(mouseS, originS, xEndS);
        if (d < best) { best = d; hotAxis = Axis::X; }
    }
    if (okY)
    {
        float d = distancePointToSegment2D(mouseS, originS, yEndS);
        if (d < best) { best = d; hotAxis = Axis::Y; }
    }
    if (okZ)
    {
        float d = distancePointToSegment2D(mouseS, originS, zEndS);
        if (d < best) { best = d; hotAxis = Axis::Z; }
    }

    // Mouse state from ImGui
    bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

    // -----------------------------
    // Begin drag
    // -----------------------------
    if (!dragging)
    {
        // Don’t start dragging if ImGui UI is already using the mouse
        // (clicking a slider, window, etc.)
        if (!uiWantsMouse && mouseClicked && hotAxis != Axis::None)
        {
            activeAxis = hotAxis;
            dragging = true;

            // Store starting object pos so movement is relative
            dragStartObjPos = selectedObject->getPosition();

            // Define the drag plane:
            // Want to move along ONE axis, but mouse movement is 2D.
            // Create a plane that:
            //  - goes through the object position
            //  - is oriented so ray hits produce stable movement along the axis
            
            // planeNormal = normalize(cross(axis, cameraForward))
            // That produces a plane that contains the axis and faces the camera.
            glm::vec3 axis = axisDir(activeAxis);
            glm::vec3 camF = camera.getFront();

            glm::vec3 planeN = glm::cross(axis, camF);
            float len = glm::length(planeN);

            // If camera forward is almost parallel to the axis,
            // cross product becomes near zero (bad plane).
            // Fallback: use camera up.
            if (len < 0.001f)
                planeN = camera.getUp();
            else
                planeN /= len;

            // Ray from mouse
            glm::vec3 ro, rd, hit;
            buildMouseRay(window, fbW, fbH, camera, ro, rd);

            // Find where the ray hits our drag plane
            if (!rayPlaneIntersection(ro, rd, dragStartObjPos, planeN, hit))
            {
                // If no hit, cancel dragging
                dragging = false;
                activeAxis = Axis::None;
                return false;
            }

            // Store initial hit point to compare later
            dragStartHitPoint = hit;

            // Return true: gizmo is now capturing mouse
            return true;
        }

        // Not dragging yet
        return false;
    }

    // -----------------------------
    // End drag
    // -----------------------------
    if (dragging && !mouseDown)
    {
        dragging = false;
        activeAxis = Axis::None;
        return false;
    }

    // -----------------------------
    // Drag update
    // -----------------------------
    if (dragging)
    {
        glm::vec3 axis = axisDir(activeAxis);
        glm::vec3 camF = camera.getFront();

        glm::vec3 planeN = glm::cross(axis, camF);
        float len = glm::length(planeN);
        if (len < 0.001f)
            planeN = camera.getUp();
        else
            planeN /= len;

        // Build ray from current mouse position
        glm::vec3 ro, rd, hit;
        buildMouseRay(window, fbW, fbH, camera, ro, rd);

        // Find new hit point on plane
        if (rayPlaneIntersection(ro, rd, dragStartObjPos, planeN, hit))
        {
            // Delta in world space between current mouse-plane hit
            // and the starting hit
            glm::vec3 delta = hit - dragStartHitPoint;

            // Project that delta onto the active axis:
            // t = dot(delta, axis)
            // This gives how far along the axis has been moved.
            float t = glm::dot(delta, axis);

            // New object position:
            // start position + axis * amount
            glm::vec3 newPos = dragStartObjPos + axis * t;
            selectedObject->setPosition(newPos);
        }

        // While dragging capture the mouse
        return true;
    }

    return false;
}

// draw():
// Renders axis lines as overlay using ImGui foreground draw list.
// This avoids needing any OpenGL rendering changes.
void EditorGizmo::draw(int fbW, int fbH, const Camera& camera, GameObject* selectedObject)
{
    if (!selectedObject) return;

    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 proj = camera.getProjectionMatrix(static_cast<float>(fbW) / fbH);

    glm::vec3 originW = selectedObject->getPosition();
    glm::vec2 originS;
    if (!worldToScreen(originW, view, proj, fbW, fbH, originS))
        return;

    // Pick axis length in world units based on camera distance
    float distToCam = glm::length(camera.getPosition() - originW);
    float axisLenWorld = glm::clamp(distToCam * 0.15f, 0.5f, 6.0f);

    // Screen endpoints
    glm::vec2 xEndS, yEndS, zEndS;
    bool okX = worldToScreen(originW + glm::vec3(1, 0, 0) * axisLenWorld, view, proj, fbW, fbH, xEndS);
    bool okY = worldToScreen(originW + glm::vec3(0, 1, 0) * axisLenWorld, view, proj, fbW, fbH, yEndS);
    bool okZ = worldToScreen(originW + glm::vec3(0, 0, 1) * axisLenWorld, view, proj, fbW, fbH, zEndS);

    // Draw on top of everything
    ImDrawList* dl = ImGui::GetForegroundDrawList();

    // Choose colors per axis:
    // X = red, Y = green, Z = blue
    // If hovered or active, tint to yellow-ish to show feedback.
    auto axisColor = [&](Axis a) -> ImU32
        {
            ImU32 base =
                (a == Axis::X) ? IM_COL32(230, 80, 80, 255) :
                (a == Axis::Y) ? IM_COL32(80, 230, 80, 255) :
                (a == Axis::Z) ? IM_COL32(80, 140, 230, 255) :
                IM_COL32(255, 255, 255, 255);

            // highlight if hovered or active
            if (dragging && a == activeAxis) return IM_COL32(255, 255, 180, 255);
            if (!dragging && a == hotAxis)   return IM_COL32(255, 255, 180, 255);
            return base;
        };

    // Draw each axis line
    if (okX)
        dl->AddLine(ImVec2(originS.x, originS.y), ImVec2(xEndS.x, xEndS.y), axisColor(Axis::X), axisLineThickness);
    if (okY)
        dl->AddLine(ImVec2(originS.x, originS.y), ImVec2(yEndS.x, yEndS.y), axisColor(Axis::Y), axisLineThickness);
    if (okZ)
        dl->AddLine(ImVec2(originS.x, originS.y), ImVec2(zEndS.x, zEndS.y), axisColor(Axis::Z), axisLineThickness);

    // Draw a small center dot so origin is visible
    dl->AddCircleFilled(ImVec2(originS.x, originS.y), 4.0f, IM_COL32(240, 240, 240, 255));
}
