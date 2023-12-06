#pragma once

#include <asset-loader.hpp>
#include <ecs/world.hpp>
#include <components/camera.hpp>
#include <components/mesh-renderer.hpp>
#include <application.hpp>

// This is a helper function that will search for a component and will return the first one found
template<typename T>
T* find(portal::World *world){
    for(const auto&  [name, entity]: world->getEntities()){
        T* component = entity->getComponent<T>();
        if(component) return component;
    }
    return nullptr;
}

// This state tests and shows how to use the ECS framework and deserialization.
class EntityTestState: public portal::State {

    portal::World world;
    
    void onInitialize() override {
        // First of all, we get the scene configuration from the app config
        auto& config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if(config.contains("assets")){
            portal::deserializeAllAssets(config["assets"]);
        }

        // If we have a world in the scene config, we use it to populate our world
        if(config.contains("world")){
            world.deserialize(config["world"]);
        }
        
    }

    void onDraw(double deltaTime) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // First, we look for a camera and if none was found, we return (there is nothing we can render)
        portal::CameraComponent* camera = find<portal::CameraComponent>(&world);
        if(camera == nullptr) return;

        // Then we compute the VP matrix from the camera
        glm::ivec2 size = getApp()->getFrameBufferSize();
        //TODO: (Req 8) Change the following line to compute the correct view projection matrix 
        glm::mat4 VP = camera->getProjectionMatrix(size) * camera->getViewMatrix();
        for(const auto&  [name, entity]: world.getEntities()){
            // For each entity, we look for a mesh renderer (if none was found, we skip this entity)
            portal::MeshRendererComponent* meshRenderer = entity->getComponent<portal::MeshRendererComponent>();
            if(meshRenderer == nullptr) continue;
            //TODO: (Req 8) Complete the loop body to draw the current entity
            // Then we setup the material, send the transform matrix to the shader then draw the mesh
            meshRenderer->material->setup();
            meshRenderer->material->shader->set("transform",VP * entity->getLocalToWorldMatrix());
            meshRenderer->mesh->draw();
        }
    }

    void onDestroy() override {
        world.clear();
        portal::clearAllAssets();
    }
};