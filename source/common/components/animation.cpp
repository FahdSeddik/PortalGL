#include "animation.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"
#include "rigidbody.hpp"
#include "../ecs/world.hpp"
#include <reactphysics3d/reactphysics3d.h>
#include <iostream>
namespace r3d = reactphysics3d;

namespace portal {
    // Reads animation data from the given json object
    void AnimationComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        start.deserialize(data.value("start", nlohmann::json::object()));
        end.deserialize(data.value("end", nlohmann::json::object()));
        duration = data.value("duration", duration);
        name = data.value("name", name);
        // For callback it would either call another animation or
        // it would call disable collider of parent of owner
        if(data.contains("callback")){
            // if type is "animation" then it would call another animation
            // if type is "disable" then it would disable collider of parent of owner
            std::string type = data["callback"].value("type", "disable");
            if(type == "animation"){
                //there can be multiple names for callbacks
                std::vector<std::string> names = data["callback"].value("names", std::vector<std::string>());
                callback = [this, names](){
                    for(auto& name : names){
                        this->getOwner()->getWorld()->startAnimation(name);
                    }
                };
            } else if(type == "disable" && this->getOwner()->parent){
                callback = [this](){
                    this->getOwner()->parent->getComponent<RigidBodyComponent>()->getCollider()->setIsTrigger(true);
                    this->getOwner()->parent->getComponent<RigidBodyComponent>()->getBody()->enableGravity(false);
                };
            }
        }

        this->getOwner()->getWorld()->addAnimation(name, this);
    }
    // Resets the animation
    void AnimationComponent::reset(){
        isPlaying = false;
        isStarted = false;
        accumulatedTime = 0.0f;
    }
    // Plays animation given delta time
    bool AnimationComponent::play(float deltaTime){
        if(!isPlaying) return false;
        if(!isStarted) {
            isStarted = true;
            accumulatedTime = 0.0f;
            this->getOwner()->localTransform.setTransform(start.getTransform());
        }
        accumulatedTime += deltaTime;
        float t = accumulatedTime / duration;
        if(t > 1.0f){
            t = 1.0f;
            reset();
            this->getOwner()->localTransform.setTransform(end.getTransform());
            if(callback) callback();
            return true;
        }
        this->getOwner()->localTransform.setTransform(Transform::interpolate(start, end, t));
        return false;
    }
}