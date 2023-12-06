#pragma once
#include <reactphysics3d/reactphysics3d.h>
#include "../../states/play-state.hpp"
#include "../deserialize-utils.hpp"
namespace r3d = reactphysics3d;

namespace portal {
    
    // Class Handle player Grounded
    class RayCastPlayerGrounded : public r3d::RaycastCallback {
        bool& isGrounded;
        public:
        RayCastPlayerGrounded(bool& isGrounded) : isGrounded(isGrounded) {}
        /// Called when a raycast hit a body
        /**
         * @param hit The raycast hit information
         */
        virtual r3d::decimal notifyRaycastHit(const r3d::RaycastInfo& raycastInfo) override {
            // is grounded 
            // std::cout << "Player Grounded: " << *((std::string*)raycastInfo.body->getUserData()) << std::endl;
            if(raycastInfo.body->getCollider(0)->getIsTrigger()){
                // if trigger, return 1.0 to continue raycast
                return r3d::decimal(1.0);
            }
            isGrounded = true;
            // return 0 to stop raycast
            return r3d::decimal(0.0);
        }
    };


    class EventSystem : public r3d::EventListener {
        r3d::PhysicsWorld* physicsWorld;
        World* world;
        bool& isGrounded;
        typedef r3d::CollisionCallback::ContactPair::EventType EventType;
        typedef std::function<void()> EventCallback;
        std::unordered_map<std::string, std::unordered_map<std::string, std::pair<int, std::pair<EventType, EventCallback>>>> collisionEvents;

        void checkForGround(const r3d::CollisionCallback::ContactPair& contactPair, const std::string& name_1) {
            RayCastPlayerGrounded rayCastHandler(isGrounded);
            // get player position and bounds
            r3d::Vector3 pos, min, max;
            if(name_1=="Player") {
                pos = contactPair.getBody1()->getTransform().getPosition();
                contactPair.getCollider1()->getCollisionShape()->getLocalBounds(min, max);
            } else {
                pos = contactPair.getBody2()->getTransform().getPosition();
                contactPair.getCollider2()->getCollisionShape()->getLocalBounds(min, max);
            }
            // RayCast to check if player is grounded
            r3d::decimal y = max.y - min.y;
            // Ray Cast from center of player to bottom of player
            r3d::Ray ray(pos, pos - r3d::Vector3(0, y/2, 0));
            physicsWorld->raycast(ray, &rayCastHandler);
        }

        public:
        EventSystem(World* world, bool& isGrounded) : isGrounded(isGrounded) {
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
                if(name_1.empty() || name_2.empty() || type.empty()) continue;
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
                    collisionEvents[name_1][name_2] = std::make_pair(once ? 1:-1, std::make_pair(eventType, callback));
                    collisionEvents[name_2][name_1] = std::make_pair(once ? 1:-1, std::make_pair(eventType, callback));
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
                // We need to cast ray if collision with player to check if player is grounded
                // We check on ContactStay to avoid isGrounded to be out of sync
                if(contactPair.getEventType() == EventType::ContactStay){
                    // Any collision with player we need to update isGrounded
                    if(name_1 == "Player" || name_2 == "Player") {
                        checkForGround(contactPair, name_1);
                    }
                }
                // Call the callback if exists
                // if the name of the body is not in the map, it means that it doesn't have any callback
                if(collisionEvents.find(name_1) == collisionEvents.end()) continue;
                // if the name exists but second body doesnt then it doesnt have interaction with this body
                if(collisionEvents[name_1].find(name_2) == collisionEvents[name_1].end()) continue;
                // in map we store redundant reverse to not have to check for order of bodies
                auto& event = collisionEvents[name_1][name_2];
                auto& eventCopy = collisionEvents[name_2][name_1];
                // if event (stay, start, exit) doesnt match then we dont call callback
                if(event.second.first != contactPair.getEventType() || event.first == 0) continue;
                // if event is once then we decrement uses to make it 0
                if(event.first > 0) event.first--, eventCopy.first--;
                // call callback
                event.second.second();
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
                if(overlapPair.getEventType() == r3d::OverlapCallback::OverlapPair::EventType::OverlapStart) {
                    std::cout << "\nOnTrigger: ";
                    std::cout << *((std::string *)overlapPair.getBody1()->getUserData());
                    std::cout << " and ";
                    std::cout << *((std::string *)overlapPair.getBody2()->getUserData());
                    std::cout << std::endl;
                }
            }
        }
    };
}