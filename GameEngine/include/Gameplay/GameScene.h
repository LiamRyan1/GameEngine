#pragma once
class Scene;
class Camera;
class Physics;
void SetupGameScene(Scene& scene, Camera& camera, Physics& physics);
// Call this after loadFromFile() to re-attach scripts to loaded objects
void SetupScripts(Scene& scene, Camera& camera, Physics& physics);