#pragma once
#include <reactphysics3d/reactphysics3d.h>
#include "../../states/play-state.hpp"
#include "../deserialize-utils.hpp"
#include "../ecs/portal.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "../ecs/entity-factory.hpp"
#include "../ecs/button.hpp"
#include <glm/gtc/matrix_access.inl>
namespace r3d = reactphysics3d;

namespace portal {
    
    class EventSystem : public r3d::EventListener {
        r3d::PhysicsWorld* physicsWorld;
        World* world;
        // Handles and stores last tp time for each object to prevent teleportation spam
        // and bugs with teleportation
        std::unordered_map<std::string, double> lastTPtime;
        // Cooldown for teleportation for each object
        double teleportationCooldown = 0.2;
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<double>>> lastCallTime;
        double sameCallCooldown = 0.2;
        typedef r3d::CollisionCallback::ContactPair::EventType EventType;
        typedef std::function<void()> EventCallback;
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::pair<int, std::pair<EventType, EventCallback>>>>> collisionEvents;


        void handleTeleport(r3d::Collider* objectCollider, const std::string& objectName, const std::string& portalName) {
            // Get portal entity and check if it is valid
            Portal* portal = dynamic_cast<Portal*>(world->getEntityByName(portalName));
            // Check if portal is valid
            if (!portal) return;
            if (!portal->destination || !portal->surface || !portal->surfaceCollider) return;
            // if object is surface then do nothing
            if(portal->surface->name == objectName) return;
            // if object is in cooldown make sure other portal
            // does not have it included to prevent issues with multithreading & thread safety
            if(lastTPtime.count(objectName) && glfwGetTime() - lastTPtime[objectName] < teleportationCooldown) return;
            // if object is not in cooldown then add to passing and update last tp time
            if(portal->addToPassing(objectCollider, objectName)) lastTPtime[objectName] = glfwGetTime();
        }

        void checkButtonCollision(Entity* entity_1, Entity* entity_2, EventType eventType) const {
            // If one of the entities is a button
            // Check if the other entity is a player or a cube
            // If so then call the press/release function of the button
            if(entity_1->getType() == EntityFactory::EntityType::Button && 
                (entity_2->getType() == EntityFactory::EntityType::Player || entity_2->getType() == EntityFactory::EntityType::Cube)) {
                if(eventType == EventType::ContactStart) {
                    dynamic_cast<Button *>(entity_1)->press();
                } else if(eventType == EventType::ContactExit) {
                    dynamic_cast<Button *>(entity_1)->release();
                }
            }
            if(entity_2->getType() == EntityFactory::EntityType::Button &&
                (entity_1->getType() == EntityFactory::EntityType::Player || entity_1->getType() == EntityFactory::EntityType::Cube)) {
                if(eventType == EventType::ContactStart) {
                    dynamic_cast<Button *>(entity_2)->press();
                } else if(eventType == EventType::ContactExit) {
                    dynamic_cast<Button *>(entity_2)->release();
                }
            }
        }

        public:
        EventSystem(World* world) {
            this->world = world;
            this->physicsWorld = world->getPhysicsWorld();
        }

        void deserialize(const nlohmann::json& data) {
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
                std::string onContact = event.value("onContact", "Start");
                EventType eventType;
                if(onContact == "Start") {
                    eventType = EventType::ContactStart;
                } else if(onContact == "Stay") {
                    eventType = EventType::ContactStay;
                } else if(onContact == "Exit") {
                    eventType = EventType::ContactExit;
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
                    collisionEvents[name_1][name_2].emplace_back(std::make_pair(once ? 1:-1, std::make_pair(eventType, callback)));
                    collisionEvents[name_2][name_1].emplace_back(std::make_pair(once ? 1:-1, std::make_pair(eventType, callback)));
                }
            }
            // resize lastCallTime
            for(auto& [name_1, map] : collisionEvents) {
                for(auto& [name_2, vector] : map) {
                    lastCallTime[name_1][name_2].resize(vector.size());
                    lastCallTime[name_2][name_1].resize(vector.size());
                }
            }
        }
        /// Called when some contacts occur
        /**
         * @param callbackData Contains information about all the contacts
         */
        virtual void onContact(const r3d::CollisionCallback::CallbackData& callbackData) override {
            // This function would get called automatically when a contact occurs between two colliders
            // this excludes trigger colliders
            for(uint32_t p = 0; p < callbackData.getNbContactPairs(); p++) {
                // For each of the contact pairs
                // contactPair.getBody1() && contactPair.getBody2() are the two bodies that are in contact
                // contactPair.getCollider1() contactPair.getCollider2() are the two colliders that are in contact
                // contactPair.getEventType() is the type of event (ContactStart/ContactStay/ContactExit)
                // contactPair.getNbContactPoints() gets the number of contact points (they can be multiple)
                // Use contactPair.getContactPoint(index) to get the contact point at the given index
                r3d::CollisionCallback::ContactPair contactPair = callbackData.getContactPair(p);
                // Get the name of the body (we stored the name in the user data while creating the body)
                // User data is stored as a void pointer so we need to cast it to the correct type
                std::string name_1 = *((std::string*)contactPair.getBody1()->getUserData());
                std::string name_2 = *((std::string*)contactPair.getBody2()->getUserData());
                Entity* entity_1 = world->getEntityByName(name_1);
                Entity* entity_2 = world->getEntityByName(name_2);

                // Check if any of the entities is a button and handle it
                checkButtonCollision(entity_1, entity_2, contactPair.getEventType());

                // Call the callback if exists
                // if the name of the body is not in the map, it means that it doesn't have any callback
                if(collisionEvents.find(name_1) == collisionEvents.end() && collisionEvents.find(name_2) == collisionEvents.end()) continue;
                // apply event for each body if it exists
                // if the name exists but second body doesnt then it doesnt have interaction with this body
                if(collisionEvents[name_1].find(name_2) == collisionEvents[name_1].end() && collisionEvents[name_2].find(name_1) == collisionEvents[name_2].end()) continue;
                // in map we store redundant reverse to not have to check for order of bodies
                auto& event = collisionEvents[name_1][name_2];
                auto& eventCopy = collisionEvents[name_2][name_1];
                for (int i = 0; i < event.size(); i++) {
                    // if event (stay, start, exit) doesnt match then we dont call callback
                    if(event[i].second.first != contactPair.getEventType() || event[i].first == 0) continue;
                    // Check cooldown on callback
                    if(glfwGetTime() - lastCallTime[name_1][name_2][i] < sameCallCooldown) continue;
                    lastCallTime[name_1][name_2][i] = glfwGetTime();
                    lastCallTime[name_2][name_1][i] = glfwGetTime();
                    // if event is once then we decrement uses to make it 0
                    if(event[i].first > 0) event[i].first--, eventCopy[i].first--;
                    // call callback
                    event[i].second.second();
                }
            }
        }

        /// Called when some trigger events occur
        /**
         * @param callbackData Contains information about all the triggers that are colliding
         */
        virtual void onTrigger(const r3d::OverlapCallback::CallbackData &callbackData) override {
            // This function would get called automatically when a trigger collider 
            // overlaps with another collider
            for (uint32_t p = 0; p < callbackData.getNbOverlappingPairs(); p++) {
                // For each of the overlapping pairs of colliders
                r3d::OverlapCallback::OverlapPair overlapPair = callbackData.getOverlappingPair(p);
                // overlapPair.getBody1() and overlapPair.getBody2() are the two colliders that are overlapping
                // overlapPair.getEventType() is the type of event (OverlapStart/OverlapStay/OverlapExit)
                std::string name_1 = *((std::string*)overlapPair.getBody1()->getUserData());
                std::string name_2 = *((std::string*)overlapPair.getBody2()->getUserData());
                if (name_1=="Portal_1" || name_1=="Portal_2") {
                    // if name_1 is a portal then name_2 is object
                    handleTeleport(overlapPair.getBody2()->getCollider(0), name_2, name_1);
                } else if(name_2=="Portal_1" || name_2=="Portal_2") {
                    // same case but reversed
                    handleTeleport(overlapPair.getBody1()->getCollider(0), name_1, name_2);
                }
            }
        }
    };
}