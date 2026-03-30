#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>
#include <functional>                 
#include <unordered_map>               
#include <string> 
#include "../include/Scene/GameObject.h"
#include "../include/Physics/Physics.h"
#include "../include/Physics/SpatialGrid.h" 
#include "../include/Rendering/Renderer.h"
enum class EngineMode;

class Scene {

private:
    Physics& physicsWorld;
    Renderer& renderer;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    // Spatial grid for fast proximity queries
    std::unique_ptr<SpatialGrid> spatialGrid;
    // Flag to track if we've synced with the editor at least once (to avoid redundant syncs)
    bool editorSyncedOnce = false;
    
    // Simple directional light state for now
    glm::vec3 savedLightDir = glm::vec3(0.3f, -1.0f, 0.5f);
    glm::vec3 savedLightCol = glm::vec3(1.0f);
    float     savedLightIntensity = 0.8f;


    std::vector<GameObject*> pendingDestroy;

    // Maps tag strings to script-attacher lambdas.
    // Populated by registerTagScript() in SetupScripts().
    // Queried by wireTagCallback() which is installed on every spawned object.
    struct TagScriptBinding {
        std::function<void(GameObject*)> attach;
        std::function<void(GameObject*)> remove;

        TagScriptBinding() = default;
        TagScriptBinding(
            std::function<void(GameObject*)> a,
            std::function<void(GameObject*)> r)
            : attach(std::move(a)), remove(std::move(r)) {
        }
    };
    std::unordered_map<std::string, TagScriptBinding> tagScriptRegistry;

    // Wires the tag->script callback into a freshly spawned object.
    // Called at the end of spawnObject(), spawnRenderObject(), and loadAndSpawnModel().
    void wireTagCallback(GameObject* obj);

    // Stores the three possible callbacks (enter, exit, stay) that a behaviour tag maps to
    // registerTriggerScript() populates this
    // applyTriggerScriptsToExistingTriggers() reads from it.
    struct TriggerScriptBinding {
        std::function<void(Trigger*)>         onRegister; // called once to wire callbacks onto the trigger
        std::function<void(Trigger*)>         onUnregister;

        TriggerScriptBinding() = default;
        TriggerScriptBinding(
            std::function<void(Trigger*)> reg,
            std::function<void(Trigger*)> unreg = nullptr)
            : onRegister(std::move(reg)), onUnregister(std::move(unreg)) {
        }
    };

    std::unordered_map<std::string, TriggerScriptBinding> triggerScriptRegistry;
    

public:
    Scene(Physics& physics, Renderer& renderer);
    ~Scene();

    // Factory methods for creating objects
    // temporary methods - Remove when loading from files
    //GameObject* createCube(const glm::vec3& position, const glm::vec3& size, float mass, const std::string& materialName = "Default", const std::string& texturePath = "");
    //GameObject* createSphere(const glm::vec3& position, float radius, float mass, const std::string& materialName = "Default", const std::string& texturePath = "");

	//generic object creation method
    GameObject* spawnObject(ShapeType type,
        const glm::vec3& position,
        const glm::vec3& size,
        float mass,
        const std::string& materialName = "Default",
		const std::string& texturePath = "",
        const std::string & specularPath = ""
    );

    // Spawn render-only object (no physics)
    GameObject* spawnRenderObject(
        ShapeType type,
        const glm::vec3& position,
        const glm::vec3& size,
        const std::string& texturePath = "",
        const std::string & specularPath = ""
    );

    void setObjectScale(GameObject* obj, const glm::vec3& newScale);
    void setObjectPhysicsScale(GameObject* obj, const glm::vec3& newPhysicsScale);
    // Update all objects from physics simulation
    void update(EngineMode mode);

    // Get all objects for rendering
    const std::vector<std::unique_ptr<GameObject>>& getObjects() const { return gameObjects; }

    // Lighting control
    void setLightState(const glm::vec3& dir, const glm::vec3& col, float intensity) {
        savedLightDir = dir;
        savedLightCol = col;
        savedLightIntensity = intensity;
    }
    void getLightState(glm::vec3& dir, glm::vec3& col, float& intensity) const {
        dir = savedLightDir;
        col = savedLightCol;
        intensity = savedLightIntensity;
    }


	// Spatial Queries

    /**
     * Find all objects within radius.
     * @param filter Optional lambda to filter results
     */
    std::vector<GameObject*> findObjectsInRadius(
        const glm::vec3& center,
        float radius,
        std::function<bool(GameObject*)> filter = nullptr
    ) const;

    /**
     * Find nearest object within max radius.
     */
    GameObject* findNearestObject(
        const glm::vec3& position,
        float maxRadius,
        std::function<bool(GameObject*)> filter = nullptr
    ) const;

    /**
     * Find all objects that have a specific tag.
     * for  SetupScripts() to bulk-attach scripts:
     *   for (auto* obj : scene.findObjectsByTag("enemy"))
     *       obj->addScript<EnemyAI>();
     */
    std::vector<GameObject*> findObjectsByTag(const std::string& tag) const;

    // When any object has this tag added (at spawn time or runtime via the Inspector),
    // the provided lambda is called immediately with that object as the argument.
    // Call this in SetupScripts() before loading/spawning objects so pre-tagged
    // objects also get their scripts attached on load.
    //   scene.registerTagScript("player", [&](GameObject* obj) {
    //   obj->addScript<PlayerController>(&camera, &physics.getQuerySystem());
    void registerTagScript(const std::string& tag, std::function<void(GameObject*)> scriptAttacher,std::function<void(GameObject*)> scriptRemover = nullptr);
    // in the scene.Call this at the end of SetupScripts() to catch objects that were
    // loaded from file before registerTagScript() was called.
    void applyTagScriptsToExistingObjects();

    void registerTriggerScript(const std::string& behaviourTag,std::function<void(Trigger*)> onRegister,std::function<void(Trigger*)> onUnregister = nullptr);
    void applyTriggerScriptsToExistingTriggers();
    // === Spatial Grid Control ===

    void setSpatialGridEnabled(bool enabled);
    bool isSpatialGridEnabled() const { return spatialGrid != nullptr; }
    void printSpatialStats() const;

    
    void requestDestroy(GameObject* obj);


    // Clear all objects
    void clear();

    // Scene serialization
    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);


    /**
     * @brief Load .obj model and spawn it in the scene
     * @param filepath Path to .obj file
     * @param position Spawn position
     * @param meshSscale  Visual mesh  scale
     * @param enablePhysics Whether to add physics
     * @param mass Mass for rigid body (0 = static)
     * @param physicsBoxScale Scale factor for collision box (0.9 = 90% of model size)
     * @param materialName Physics material name
     * @return Pointer to spawned GameObject
     */
    GameObject* loadAndSpawnModel(const std::string& filepath,
        const glm::vec3& position,
        const glm::vec3& meshScale,
        bool enablePhysics,
        float mass,
        const glm::vec3& physicsBoxScale,
        const std::string& materialName
    );

   

};

#endif