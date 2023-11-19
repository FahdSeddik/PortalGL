#pragma once

#include <asset-loader.hpp>
#include <ecs/world.hpp>
#include <components/camera.hpp>
#include <components/mesh-renderer.hpp>
#include <systems/forward-renderer.hpp>
#include <application.hpp>

// This state tests and shows how to use the Forward renderer.
class RendererTestState: public portal::State {

    portal::World world;
    portal::ForwardRenderer renderer;
    
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

        glm::ivec2 size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
    }

    void onDraw(double deltaTime) override {
        // We simply call the renderer's "render" function and it should do all the rendering work
        renderer.render(&world);
    }

    void onDestroy() override {
        world.clear();
        portal::clearAllAssets();
    }
};