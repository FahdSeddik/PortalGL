#include "event.hpp"
#include "../ecs/entity-factory.hpp"
#include "../ecs/button.hpp"
#include "../ecs/elevator.hpp"
#include "../ecs/portal.hpp"
#include "../deserialize-utils.hpp"

namespace portal {
    void EventSystem::handleTeleport(r3d::Collider* objectCollider, Entity* object, Portal* portal) {
        // Check if portal is valid
        if (!portal) return;
        if (!portal->destination || !portal->surface || !portal->surfaceCollider) return;
        // if object is surface then do nothing
        if(portal->surface->name == object->name) return;
        // if object is in cooldown make sure other portal
        // does not have it included to prevent issues with multithreading & thread safety
        if(lastTPtime.count(object->name) && glfwGetTime() - lastTPtime[object->name] < teleportationCooldown) return;
        // if object is not in cooldown then add to passing and update last tp time
        if(portal->addToPassing(objectCollider, object->name)) lastTPtime[object->name] = glfwGetTime();
    }

    void EventSystem::checkButtonCollision(Entity* entity_1, Entity* entity_2, CollisionType eventType) const {
        // If one of the entities is a button
        // Check if the other entity is a player or a cube
        // If so then call the press/release function of the button
        if(entity_1->getType() == EntityFactory::EntityType::Button && 
            (entity_2->getType() == EntityFactory::EntityType::Player || entity_2->getType() == EntityFactory::EntityType::Cube)) {
            if(eventType == CollisionType::ContactStart) {
                dynamic_cast<Button *>(entity_1)->press();
            } else if(eventType == CollisionType::ContactExit) {
                dynamic_cast<Button *>(entity_1)->release();
            }
        }
        if(entity_2->getType() == EntityFactory::EntityType::Button &&
            (entity_1->getType() == EntityFactory::EntityType::Player || entity_1->getType() == EntityFactory::EntityType::Cube)) {
            if(eventType == CollisionType::ContactStart) {
                dynamic_cast<Button *>(entity_2)->press();
            } else if(eventType == CollisionType::ContactExit) {
                dynamic_cast<Button *>(entity_2)->release();
            }
        }
    }

    void EventSystem::checkElevatorTrigger(Entity* entity_1, Entity* entity_2) const {
        // If one of the entities is an elevator
        // Check if the other entity is a player or a cube
        // If so then call the press/release function of the elevator
        if(entity_1->getType() == EntityFactory::EntityType::Elevator && entity_2->getType() == EntityFactory::EntityType::Player) {
            dynamic_cast<Elevator *>(entity_1)->close();
        }
        if(entity_2->getType() == EntityFactory::EntityType::Elevator && entity_1->getType() == EntityFactory::EntityType::Player) {
            dynamic_cast<Elevator *>(entity_2)->close();
        }
    }


    void EventSystem::deserialize(const nlohmann::json& data) {
        if(!data.is_array()) return;
        for(const auto& event : data) {
            if(!event.is_object()) continue;
            // get names of bodies
            std::string name_1 = event.value("body1", "");
            std::string name_2 = event.value("body2", "");
            // get type of event (animation for example)
            std::string type = event.value("type", "");
            // if any of the names or type is empty then we skip this event (not valid)
            if((name_1.empty() || name_2.empty()) || type.empty()) continue;
            // get if event callback is called once or multiple times
            bool once = event.value("once", true);
            // get when event is called (start, stay, exit)
            std::string onContact = event.value("onOverlap", "Start");
            EventType eventType;
            if(onContact == "Start") {
                eventType = EventType::OverlapStart;
            } else if(onContact == "Stay") {
                eventType = EventType::OverlapStay;
            } else if(onContact == "Exit") {
                eventType = EventType::OverlapExit;
            } else {
                // if not valid then we skip this event
                continue;
            }
            if(type == "animation") {
                // get animation names
                std::vector<std::string> animations = event.value("names", std::vector<std::string>());
                EventCallback callback = [animations, this]() {
                    for(const auto& name : animations) {
                        if(name[0] == '#') {
                            // Start animation in reverse
                            world->startAnimation(name.substr(1), true);
                        } else {
                            world->startAnimation(name);
                        }
                    }
                };
                // add callback to map with reverse for time optimization
                // Then this means that name_2 with any other body
                triggerEvents[name_1][name_2].emplace_back(std::make_pair(once ? 1:-1, std::make_pair(eventType, callback)));
                triggerEvents[name_2][name_1].emplace_back(std::make_pair(once ? 1:-1, std::make_pair(eventType, callback)));
            }
        }
        // resize lastCallTime
        for(auto& [name_1, map] : triggerEvents) {
            for(auto& [name_2, vector] : map) {
                lastCallTime[name_1][name_2].resize(vector.size());
                lastCallTime[name_2][name_1].resize(vector.size());
            }
        }
    }
}