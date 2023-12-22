#include "world.hpp"
#include <json/json.hpp>
#include "../deserialize-utils.hpp"
#include "../components/animation.hpp"
#include "portal.hpp"
#include "../systems/event.hpp"
#include "entity-factory.hpp"
namespace portal {

    // This will deserialize a json array of entities and add the new entities to the current world
    // If parent pointer is not null, the new entities will be have their parent set to that given pointer
    // If any of the entities has children, this function will be called recursively for these children
    void World::deserialize(const nlohmann::json& data, Entity* parent){
        if(!data.is_array()) return;
        for(const auto& entityData : data){
            std::string name = entityData.value("name", std::to_string(entities.size()));
            std::string type = entityData.value("type", "Regular");
            Entity *entity = EntityFactory::createEntity(EntityFactory::stringToEntityType(type));
            entity->parent = parent;
            entity->name = name;
            entity->world = this;
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

    void World::deserialize_physics(const nlohmann::json& data, const nlohmann::json* onTriggerData){
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
        // Deserialize onTriggerEvents if exists
        if(onTriggerData)eventSystem->deserialize(*onTriggerData);
        this->physicsWorld->setEventListener(eventSystem);
    }
    
    void World::startAnimation(const std::string& name, bool reverse) {
        // Checks if the given animation isn't already playing and exists
        if(animations.find(name) != animations.end() && playingAnimations.find(name) == playingAnimations.end()) {
            std::vector<std::string> names;
            for(auto& [animName, anim]: playingAnimations) {
                if(anim->getOwner() == animations.at(name)->getOwner()) {
                    names.push_back(animName);
                }
            }
            for(auto& animName : names) {
                AnimationComponent* animcomp = playingAnimations.at(animName);
                playingAnimations.erase(animName);
            }
            // Start playing the animation
            animations[name]->startPlaying(reverse);
            // Add this animation to playingAnimations map
            playingAnimations[name] = animations[name];
        } else if (playingAnimations.find(name) != playingAnimations.end()) {
            // play other animation
            animations[name]->startPlaying(reverse);
            // Add this animation to playingAnimations map
            playingAnimations[name] = animations[name];
        }
    }

    // Ensure resetting when changing states of the game
    void World::clearPlayingAnimations() {
        playingAnimations.clear();
    }

    void World::stopAnimations() {
        for(auto& name : toStopPlaying) {
            if(playingAnimations.find(name) == playingAnimations.end()) continue;
            AnimationComponent *animTemp = playingAnimations.at(name);
            playingAnimations.erase(name);
            animTemp->envokeCallback();
        }
        toStopPlaying.clear();
    }
}