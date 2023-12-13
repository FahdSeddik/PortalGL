#include "world.hpp"
#include <json/json.hpp>
#include "../deserialize-utils.hpp"
#include "../components/animation.hpp"
#include "portal.hpp"
#include "../systems/event.hpp"
namespace portal {

    // This will deserialize a json array of entities and add the new entities to the current world
    // If parent pointer is not null, the new entities will be have their parent set to that given pointer
    // If any of the entities has children, this function will be called recursively for these children
    void World::deserialize(const nlohmann::json& data, Entity* parent){
        if(!data.is_array()) return;
        for(const auto& entityData : data){
            bool isportal = entityData.value("isPortal", false);
            Entity *entity;
            if(isportal) {
                entity = new Portal();
                entity->world = this;
            } else {
                entity = add();
            }
            entity->parent = parent;
            entity->name = entityData.value("name", std::to_string(entities.size()));
            entities[entity->name] = entity;
            entity->deserialize(entityData);
            if(entityData.contains("children")){
                deserialize(entityData["children"], entity);
            }
        }
    }

    Entity *World::getEntityByName(const std::string &name) const {
        return entities.at(name);
    }

    void World::deserialize_physics(const nlohmann::json& data, const nlohmann::json* OnCollisionData){
        if(!data.is_object()) return;
        r3d::PhysicsWorld::WorldSettings settings;
        glm::vec3 gravity(settings.gravity.x, settings.gravity.y, settings.gravity.z);
        gravity = data.value("gravity", gravity);
        settings.gravity = r3d::Vector3(gravity.x, gravity.y, gravity.z);
        settings.worldName = data.value("worldName", settings.worldName);
        // TODO: Support other world settings if needed

        this->physicsWorld = this->physicsCommon.createPhysicsWorld(settings);
        // Create a new eventsystem that would be used to detect collisions
        // pass isGrounded by reference to allow the event system to change it
        EventSystem *eventSystem = new EventSystem(this);
        this->eventSystem = eventSystem;
        // Deserialize OnCollisionEvents if exists
        if(OnCollisionData)eventSystem->deserialize(*OnCollisionData);
        this->physicsWorld->setEventListener(eventSystem);
    }
    
    void World::startAnimation(const std::string& name, bool reverse) {
        // Checks if the given animation isn't already playing and exists
        if(animations.find(name) != animations.end() && playingAnimations.find(name) == playingAnimations.end()) {
            // Start playing the animation
            animations[name]->startPlaying(reverse);
            // Add this animation to playingAnimations map
            playingAnimations[name] = animations[name];
        }
    }

    // Ensure resetting when changing states of the game
    void World::clearPlayingAnimations() {
        playingAnimations.clear();
    }

    void World::initEventSystem() const {
        if(eventSystem)eventSystem->init();
    }

}