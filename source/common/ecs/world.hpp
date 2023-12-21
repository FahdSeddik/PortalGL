#pragma once

#include <unordered_set>
#include "entity.hpp"
#include <reactphysics3d/reactphysics3d.h>

namespace portal {

    class AnimationComponent;
    class EventSystem;
    // This class holds a set of entities
    class World {
        std::unordered_map<std::string, Entity*> entities; // These are the entities held by this world
        std::unordered_set<std::string> markedForRemoval; // These are the entities that are awaiting to be deleted
                                                      // when deleteMarkedEntities is called
        r3d::PhysicsCommon physicsCommon; // Factory pattern for creating physics world objects , logging, and memory management
        r3d::PhysicsWorld* physicsWorld = nullptr; // This is the physics world that will be used for physics simulation
        EventSystem* eventSystem = nullptr; // This is the event system that will be used for collision detection
        std::unordered_map<std::string, AnimationComponent*> animations;
        std::unordered_map<std::string, AnimationComponent *> playingAnimations;
        std::unordered_set<std::string> toStopPlaying;
    public:

        World() = default;

        // This will deserialize a json array of entities and add the new entities to the current world
        // If parent pointer is not null, the new entities will be have their parent set to that given pointer
        // If any of the entities has children, this function will be called recursively for these children
        void deserialize(const nlohmann::json& data, Entity* parent = nullptr);

        // This will deserialize a json object of physics world settings and create a physics world
        // The physics world will be used for physics simulation
        void deserialize_physics(const nlohmann::json& data, const nlohmann::json* OnCollisionData = nullptr);

        // This returns and immutable reference to the set of all entites in the world.
        const std::unordered_map<std::string, Entity*>& getEntities() {
            return entities;
        }

        Entity *getEntityByName(const std::string &name) const;

        // This marks an entity for removal by adding it to the "markedForRemoval" set.
        // The elements in the "markedForRemoval" set will be removed and deleted when "deleteMarkedEntities" is called.
        void markForRemoval(Entity* entity){
            //TODO: (Req 8) If the entity is in this world, add it to the "markedForRemoval" set.
            // If the entity is in this world, add it to the "markedForRemoval" set.
            if (entities.find(entity->name) != entities.end()) {
                markedForRemoval.insert(entity->name);
            }
        }

        // This removes the elements in "markedForRemoval" from the "entities" set.
        // Then each of these elements are deleted.
        void deleteMarkedEntities(){
            //TODO: (Req 8) Remove and delete all the entities that have been marked for removal
            // Remove all the entities that have been marked for removal
            for (auto& name : markedForRemoval) {
                delete entities[name];
                entities.erase(name);
            }
            markedForRemoval.clear();
        }

        //This deletes all entities in the world
        void clear(){
            //TODO: (Req 8) Delete all the entites and make sure that the containers are empty
            // Add every element in entities to markForRemoval list
            for (const auto& [name, entity] : entities) {
                markForRemoval(entity);
            }
            // Call deleteMarkedEntities function
            deleteMarkedEntities();
        }

        r3d::PhysicsWorld* getPhysicsWorld() {
            return physicsWorld;
        }
        
        r3d::PhysicsCommon& getPhysicsCommon() {
            return physicsCommon;
        }

        void startAnimation(const std::string &name, bool reverse = false);

        void addAnimation(const std::string& name, AnimationComponent* animation) {
            animations[name] = animation;
        }

        AnimationComponent* getAnimationByName(const std::string& name) const {
            return animations.count(name) ? animations.at(name) : nullptr;
        }

        std::unordered_map<std::string, AnimationComponent*>& getPlayingAnimations() {
            return playingAnimations;
        }

        void markAnimationForStop(const std::string& name) {
            toStopPlaying.insert(name);
        }

        void stopAnimations();

        // Clears the playing animations map to ensure 
        // resetting when changing states of the game
        void clearPlayingAnimations();

        //Since the world owns all of its entities, they should be deleted alongside it.
        ~World(){
            clear();
            // Destroy the physics world if it exists
            if(physicsWorld) {
                physicsCommon.destroyPhysicsWorld(physicsWorld);
            }
        }

        // The world should not be copyable
        World(const World&) = delete;
        World &operator=(World const &) = delete;
    };

}