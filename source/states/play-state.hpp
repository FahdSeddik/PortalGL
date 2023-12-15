#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>
#include "../common/components/animation.hpp"
#include "systems/event.hpp"

// This state shows how to use the ECS framework and deserialization.
class Playstate: public portal::State {

    portal::World world;
    portal::ForwardRenderer renderer;
    portal::FreeCameraControllerSystem cameraController;
    portal::MovementSystem* movementSystem;

public:
    // mark constructor to not delete
    Playstate() = default;
    private:

    void onInitialize() override {
        // First of all, we get the scene configuration from the app config
        auto& config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if(config.contains("assets")){
            portal::deserializeAllAssets(config["assets"]);
        }
        if(config.contains("physicsWorld")){
            world.deserialize_physics(config["physicsWorld"], config.contains("onCollisionEvents") ? &config["onCollisionEvents"] : nullptr);
        }
        // If we have a world in the scene config, we use it to populate our world
        if(config.contains("world")){
            world.deserialize(config["world"]);
            // After making sure that the world is populated, we initialize the event system
            world.initEventSystem();
        }
        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);

        movementSystem = new portal::MovementSystem(&world, getApp());
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
        if(keyboard.justPressed(GLFW_KEY_H)){
            world.startAnimation("door_1_left_spin");
            world.startAnimation("door_1_right_spin");
        }
        if(keyboard.justPressed(GLFW_KEY_J)){
            world.startAnimation("door_1_left_open", true);
            world.startAnimation("door_1_right_open", true);
            world.startAnimation("door_1_left_spin_open", true);
            world.startAnimation("door_1_right_spin_open", true);
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