#pragma once

#include "../ecs/entity.hpp"
#include "camera.hpp"
#include "mesh-renderer.hpp"
#include "free-camera-controller.hpp"
#include "movement.hpp"
#include "RigidBody.hpp"
#include "lighting.hpp"
#include "../ecs/world.hpp"
#include "../components/animation.hpp"

namespace portal {

    // Utility to deserialize ModelLoader
    inline void addModelLoader(World* world, const nlohmann::json& data, Entity* entity) {
        if(!data.is_object()) return;
        if(data.contains("model")) {
            std::string model = data["model"];
            // modelData is just a list of json objects that represent entities
            const nlohmann::json *modelData = AssetLoader<nlohmann::json>::get(model);
            if(modelData) {
                world->deserialize(*modelData, entity);
            }
        }
    }

    // Given a json object, this function picks and creates a component in the given entity
    // based on the "type" specified in the json object which is later deserialized from the rest of the json object
    inline void deserializeComponent(const nlohmann::json& data, Entity* entity){
        std::string type = data.value("type", "");
        Component* component = nullptr;
        //TODO: (Req 8) Add an option to deserialize a "MeshRendererComponent" to the following if-else statement
        if(type == CameraComponent::getID()){
            component = entity->addComponent<CameraComponent>();
        } else if (type == FreeCameraControllerComponent::getID()) {
            component = entity->addComponent<FreeCameraControllerComponent>();
        } else if (type == MovementComponent::getID()) {
            component = entity->addComponent<MovementComponent>();
        } else if(type == MeshRendererComponent::getID()) {
            component = entity->addComponent<MeshRendererComponent>();
        } else if(type == RigidBodyComponent::getID()) {
            component = entity->addComponent<RigidBodyComponent>();
        } else if(type == LightComponent::getID()) {
            component = entity->addComponent<LightComponent>();
        } else if(type == "ModelLoader") {
            addModelLoader(entity->getWorld(), data, entity);
        } else if(type == AnimationComponent::getID()) {
            component = entity->addComponent<AnimationComponent>();
        }        
        if(component) component->deserialize(data);
    }

}