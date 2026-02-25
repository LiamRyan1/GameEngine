#pragma once

class GameObject;

class ScriptComponent
{
public:
    virtual ~ScriptComponent() = default;

    void setOwner(GameObject* obj) { owner = obj; }

    virtual void update(float dt) {}

protected:
    GameObject* owner = nullptr;
};