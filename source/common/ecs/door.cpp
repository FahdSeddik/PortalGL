#include "door.hpp"
#include "world.hpp"
#include "../components/animation.hpp"
#include <json/json.hpp>
#include "../components/RigidBody.hpp"
#include <reactphysics3d/reactphysics3d.h>
namespace portal {
    void Door::setupAnimations() {
        // Need to get right_spin_open
        // and setup callback_reversed
        // Need to get left_spin
        // and setup callback
        AnimationComponent *left_spin = getWorld()->getAnimationByName(name + "_" + "left_spin");
        AnimationComponent *right_spin_open = getWorld()->getAnimationByName(name + "_" + "right_spin_open");
        if(left_spin && right_spin_open) {
            // Opening sequence
            left_spin->deserializeCallback(nlohmann::json::object({
                {"type", "animation"},
                {"names", nlohmann::json::array({name + "_" + "left_open", name + "_" + "left_spin_open", 
                                                name + "_" + "right_open", name + "_" + "right_spin_open"})}
                }).get<nlohmann::json::object_t>()
            );
            // Closing sequence
            right_spin_open->deserializeCallback(nlohmann::json::object({
                {"type", "animation"},
                {"names", nlohmann::json::array({"#" + name + "_" + "left_spin", "#" + name + "_" + "right_spin"})}
                }).get<nlohmann::json::object_t>(), true
            );
        }
        // Initialize colliders
        collider = this->getComponent<RigidBodyComponent>()->getCollider();
    }
    void Door::open() {
        if(!isOpened) {
            if(!animationSetup) {
                // If animation is not setup, setup animations
                setupAnimations();
                animationSetup = true;
            }
            // Call opening sequence
            isOpened = true;
            collider->setIsTrigger(true);
            getWorld()->startAnimation(name + "_" + "left_spin");
            getWorld()->startAnimation(name + "_" + "right_spin");
        }
    }

    void Door::close() {
        if(isOpened) {
            if(!animationSetup) {
                // If animation is not setup, setup animations
                setupAnimations();
                animationSetup = true;
            }
            // Call closing sequence
            isOpened = false;
            collider->setIsTrigger(false);
            getWorld()->startAnimation(name + "_" + "left_open", true);
            getWorld()->startAnimation(name + "_" + "right_open", true);
            getWorld()->startAnimation(name + "_" + "left_spin_open", true);
            getWorld()->startAnimation(name + "_" + "right_spin_open", true);
        }
    }
}