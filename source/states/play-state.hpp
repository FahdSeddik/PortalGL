#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>
#include "../common/components/animation.hpp"
#include "systems/event.hpp"
#include "../common/loading-screen.hpp"

// This state shows how to use the ECS framework and deserialization.
class Playstate: public portal::State {

    portal::World world;
    portal::ForwardRenderer renderer;
    portal::FreeCameraControllerSystem cameraController;
    portal::MovementSystem* movementSystem;

    void loadConfig(const nlohmann::json& config) {
        // If we have assets in the scene config, we deserialize them
        if(config.contains("assets")){
            portal::deserializeAllAssets(config["assets"]);
        }
        if(config.contains("physicsWorld")){
            world.deserialize_physics(config["physicsWorld"], config.contains("onCollisionEvents") ? &config["onCollisionEvents"] : nullptr);
        }
    }

    void createWorld(const nlohmann::json& config) {
        // If we have a world in the scene config, we use it to populate our world
        if(config.contains("world")){
            world.deserialize(config["world"]);
        }
        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);

        movementSystem = new portal::MovementSystem(&world, getApp());
    }

public:
    // mark constructor to not delete
    Playstate() = default;
    private:

    void onInitialize() override {
        auto& config = getApp()->getConfig()["scene"];
        // This is an example of how to use the loading screen
        // if you want to simply load the scene
        // Note: this usage makes loaded meshes to be stored in LoadingScreen::meshData
        // and is transferred in main thread (due to argument callback = nullptr)
        portal::LoadingScreen::init(getApp(),
        [&config, this](){
            // A function that updates LoadingScreen::progress
            // Sets LoadingScreen::doneLoading to true when done
            // set AssetLoader<T>::separateThread to true to load assets in a separate thread
            portal::AssetLoader<portal::Mesh>::separateThread = true;
            glfwMakeContextCurrent(getApp()->getSharedWindow());
            loadConfig(config);
            portal::AssetLoader<portal::Mesh>::separateThread = false;
        },
        [&config](){
            // This function should be responsible to compute
            // LoadingScreen::total (total number to count in progress bar)
            portal::LoadingScreen::countTotalAssets(config["assets"]);
        }, nullptr);
        portal::LoadingScreen::render();
        createWorld(config);
    }

    void onDraw(double deltaTime) override {
        // Here, we just run a bunch of systems to control the world logic
        movementSystem->update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        
        // Loop on playing animations and play them given delta time
        for(auto& animation : world.getPlayingAnimations()){
            if(animation.second->play((float)deltaTime)){
                // If return true then animation finished then we need to mark for stop
                world.markAnimationForStop(animation.first);
            }
        }
        // Stop animations marked for stop
        world.stopAnimations();

        // And finally we use the renderer system to draw the scene
        renderer.render(&world);

        // Get a reference to the keyboard object
        auto& keyboard = getApp()->getKeyboard();

        if(keyboard.justPressed(GLFW_KEY_ESCAPE)){
            // If the escape  key is pressed in this frame, go to the play state
            getApp()->changeState("menu");
            world.clearPlayingAnimations();
        }
    }

    void onDestroy() override {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        // Clear the world
        world.clear();
        // and we delete all the loaded assets to free memory on the RAM and the VRAM
        portal::clearAllAssets();
    }
};