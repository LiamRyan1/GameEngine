#pragma once

class GameObject;


//  ScriptComponent  -  ENGINE SIDE (do not modify)
//
//  Base class for all user gameplay scripts.
//  Put your game logic in src/Gameplay/ by inheriting from this.
//
//  Lifecycle:
//    onStart()        - called once when the script is first attached
//    onUpdate(dt)     - called every frame in Game mode
//    onFixedUpdate()  - called every physics tick (1/60s) in Game mode
//    onDestroy()      - called when the owning GameObject is destroyed
//



class ScriptComponent
{
public:
    virtual ~ScriptComponent() = default;

    // Called by GameObject::addScript() immediately after attachment
    virtual void onStart() {}

    // Called every frame (variable dt) in Game mode
    virtual void onUpdate(float dt) {}

    // Called every physics tick (fixed 1/60s) in Game mode
    virtual void onFixedUpdate(float fixedDt) {}

    // Called when the owning GameObject is about to be destroyed
    virtual void onDestroy() {}

    void setOwner(GameObject* obj) { owner = obj; }
    GameObject* getOwner() const { return owner; }

protected:
    GameObject* owner = nullptr;
};