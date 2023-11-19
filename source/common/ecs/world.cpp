#include "world.hpp"
#include <json/json.hpp>
#include "../deserialize-utils.hpp"

namespace portal {

    // This will deserialize a json array of entities and add the new entities to the current world
    // If parent pointer is not null, the new entities will be have their parent set to that given pointer
    // If any of the entities has children, this function will be called recursively for these children
    void World::deserialize(const nlohmann::json& data, Entity* parent){
        if(!data.is_array()) return;
        for(const auto& entityData : data){
            Entity* entity = add();
                entity->parent = parent;
            entity->deserialize(entityData);
            if(entityData.contains("children")){
                deserialize(entityData["children"], entity);
            }
        }
    }

    void World::deserialize_physics(const nlohmann::json& data){
        if(!data.is_object()) return;
        r3d::PhysicsWorld::WorldSettings settings;
        glm::vec3 gravity(settings.gravity.x, settings.gravity.y, settings.gravity.z);
        gravity = data.value("gravity", gravity);
        settings.gravity = r3d::Vector3(gravity.x, gravity.y, gravity.z);
        settings.worldName = data.value("worldName", settings.worldName);
        // TODO: Support world settings like gravity and world name

        this->physicsWorld = this->physicsCommon.createPhysicsWorld(settings);
    }

}