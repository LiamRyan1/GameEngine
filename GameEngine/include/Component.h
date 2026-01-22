#ifndef COMPONENT_H
#define COMPONENT_H

class GameObject;

/**
 * @brief Base class for all components.
 * Components are modular pieces of functionality attached to GameObjects.
 */
class Component {
protected:
    GameObject* owner;  // Non-owning pointer to parent GameObject

public:
	// constructor and destructor
    Component() : owner(nullptr) {}
	// use virtual destructor for proper cleanup of derived classes
    virtual ~Component() = default;

	// set and get owner GameObject
    void setOwner(GameObject* obj) { owner = obj; }
    GameObject* getOwner() const { return owner; }

    // update method - override if component needs per-frame logic
    virtual void update(float deltaTime) {}
};

#endif // COMPONENT_H