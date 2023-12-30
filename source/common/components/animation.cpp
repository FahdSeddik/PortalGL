#include "animation.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"
#include "rigidbody.hpp"
#include "../ecs/world.hpp"
#include <reactphysics3d/reactphysics3d.h>
#include <iostream>
namespace r3d = reactphysics3d;

namespace portal {


    void AnimationComponent::deserializeCallback(const nlohmann::json& data, bool reverse) {
        if(!data.is_object()) return;
        // if type is "animation" then it would call another animation
        std::string type = data.value("type", "");
        std::function<void()> tempCallback = nullptr;
        if(type == "animation"){
            //there can be multiple names for callbacks
            std::vector<std::string> names = data.value("names", std::vector<std::string>());
            tempCallback = [this, names](){
                // Loop on each animation name and start it
                for(auto& name : names){
                    // If first character is '#' then animation is reversed
                    if(name[0] == '#'){
                        this->getOwner()->getWorld()->startAnimation(name.substr(1), true);
                    } else {
                        this->getOwner()->getWorld()->startAnimation(name);
                    }
                }
            };
        }
        if (reverse) {
            reverseCallback = tempCallback;
        } else {
            callback = tempCallback;
        }
    }

    // Reads animation data from the given json object
    void AnimationComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        start.deserialize(data.value("start", nlohmann::json::object()));
        end.deserialize(data.value("end", nlohmann::json::object()));
        duration = data.value("duration", duration);
        name = data.value("name", name);
        // Append the name of the parent entity to the animation name
        // Since animation can only be applied to child entities
        // Make them unique that way if parent exists
        if(this->getOwner()->parent) name = this->getOwner()->parent->name + "_" + name;
        // For callback it would either call another animation or
        // it would call disable collider of parent of owner
        if(data.contains("callback")) {
            deserializeCallback(data["callback"]);
        }
        if(data.contains("callback_reversed")) {
            deserializeCallback(data["callback_reversed"], true);
        }

        this->getOwner()->getWorld()->addAnimation(name, this);
    }
    // Resets the animation
    void AnimationComponent::reset(){
        isPlaying = false;
        isStarted = false;
        isReversed = false;
        accumulatedTime = 0.0f;
    }
    // Plays animation given delta time
    bool AnimationComponent::play(float deltaTime){
        if(!isPlaying) return false;
        if(!isStarted) {
            isStarted = true;
            accumulatedTime = 0.0f;
            if (isReversed) {
                this->getOwner()->localTransform.setTransform(end.getTransform());
            } else {
                this->getOwner()->localTransform.setTransform(start.getTransform());
            }
        }
        accumulatedTime += deltaTime;
        float t = accumulatedTime / duration;
        if(t > 1.0f){
            t = 1.0f;
            if (isReversed) {
                this->getOwner()->localTransform.setTransform(start.getTransform());
            } else {
                this->getOwner()->localTransform.setTransform(end.getTransform());
            }
            return true;
        }
        if (isReversed) {
            this->getOwner()->localTransform.setTransform(Transform::interpolate(end, start, t));
        } else {
            this->getOwner()->localTransform.setTransform(Transform::interpolate(start, end, t));
        }
        return false;
    }
}