#pragma once
#include "../../states/play-state.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.inl>
#include <reactphysics3d/reactphysics3d.h>
namespace r3d = reactphysics3d;

namespace portal {
    class Portal;
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
        typedef r3d::OverlapCallback::OverlapPair::EventType EventType;
        typedef r3d::CollisionCallback::ContactPair::EventType CollisionType;
        typedef std::function<void()> EventCallback;
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::pair<int, std::pair<EventType, EventCallback>>>>> triggerEvents;

        void handleTeleport(r3d::Collider *objectCollider, Entity *object, Portal *portal);

        void checkButtonCollision(Entity *entity_1, Entity *entity_2, CollisionType eventType) const;

        void checkElevatorTrigger(Entity *entity_1, Entity *entity_2) const;

    public:
        EventSystem(World* world) {
            this->world = world;
            this->physicsWorld = world->getPhysicsWorld();
        }
        // Deserialize onTriggerEvents if exists
        void deserialize(const nlohmann::json &data);
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
                Entity* entity_1 = world->getEntityByName(name_1);
                Entity* entity_2 = world->getEntityByName(name_2);
                if (entity_1->getType() == EntityFactory::EntityType::Portal) {
                    // if name_1 is a portal then name_2 is object
                    handleTeleport(overlapPair.getBody2()->getCollider(0), entity_2, dynamic_cast<Portal*>(entity_1));
                } else if(entity_2->getType() == EntityFactory::EntityType::Portal) {
                    // same case but reversed
                    handleTeleport(overlapPair.getBody1()->getCollider(0), entity_1, dynamic_cast<Portal*>(entity_2));
                }
                // This would be used to change levels 
                // would be called if a trigger inside the elevator collides with something
                if(overlapPair.getEventType() != EventType::OverlapStay) {
                    checkElevatorTrigger(entity_1, entity_2);
                }

                // Call the callback if exists
                // if the name of the body is not in the map, it means that it doesn't have any callback
                if(triggerEvents.find(name_1) == triggerEvents.end() && triggerEvents.find(name_2) == triggerEvents.end()) continue;
                // apply event for each body if it exists
                // if the name exists but second body doesnt then it doesnt have interaction with this body
                if(triggerEvents[name_1].find(name_2) == triggerEvents[name_1].end() && triggerEvents[name_2].find(name_1) == triggerEvents[name_2].end()) continue;
                // in map we store redundant reverse to not have to check for order of bodies
                auto& event = triggerEvents[name_1][name_2];
                auto& eventCopy = triggerEvents[name_2][name_1];
                for (int i = 0; i < event.size(); i++) {
                    // if event (stay, start, exit) doesnt match then we dont call callback
                    if(event[i].second.first != overlapPair.getEventType() || event[i].first == 0) continue;
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
    };
}