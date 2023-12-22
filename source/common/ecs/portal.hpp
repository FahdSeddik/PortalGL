#pragma once
#include "entity.hpp"
#include <reactphysics3d/reactphysics3d.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>
#include <unordered_set>
namespace r3d = reactphysics3d;


namespace portal {
    class RigidBodyComponent;
    class EntityFactory;
    class Portal : public Entity {
        // Class Handle player Grounded
        class RayCastGetSurface : public r3d::RaycastCallback {
            std::string& surfaceName;
            public:
            RayCastGetSurface(std::string& surfaceName) : surfaceName(surfaceName) {}
            // Called when a raycast hits a body
            virtual r3d::decimal notifyRaycastHit(const r3d::RaycastInfo& raycastInfo) override {
                // Get the name of the body that has been hit
                if(raycastInfo.body->getCollider(0)->getIsTrigger()){
                    // if trigger, return 1.0 to continue raycast
                    return r3d::decimal(1.0);
                }
                surfaceName = *((std::string*)raycastInfo.body->getUserData());
                std::cout << "Surface Retrieved: " << surfaceName << std::endl;
                // return 0 to stop raycast
                return r3d::decimal(0.0);
            }
        };
        // Make EntityFactory a friend so that it can call the private constructor
        friend class EntityFactory;

        // half turn quaternion to be used in teleportation
        glm::fquat halfTurn = glm::angleAxis(glm::pi<float>(), glm::vec3(0, 1, 0));
        // Holds the rigidbody of one of the objects that teleportation is being performed on
        RigidBodyComponent *objectRgb = nullptr;
        // boolean to indicate whether collider of object or surface is to be disabled
        bool togObj;
        // Currently tracked objects that are colliding with portal and need to check if they passed or not
        std::unordered_map<std::string, std::shared_ptr<r3d::Collider*>> passedObjects;
        // Objects that got teleported and need to be removed from passedObjects
        std::unordered_set<std::string> markedForRemoval;
        // Fail Safe teleport location of objects in case of failure (e.g. player runs too far from portal)
        std::unordered_map<std::string, glm::vec4> failSafeTeleportLocation;
        // To project point of player onto plane of portal and store that point as failSafeTeleportLocation
        void calculateFailSafeLocation(const std::string &objectName);
        // To check if we should use failsafeLocation on object or not given object position
        bool shouldUseFailSafeLocation(const r3d::Vector3 &objectPosition) const;

        // Given the position of an object, calculate the position of the object after teleportation
        r3d::Vector3 teleportedPosition(const std::string& ObjectName, const r3d::Vector3 &objectPosition) const;
        // Given the rotation of an object, calculate the rotation of the object after teleportation
        r3d::Quaternion teleportedRotation(const r3d::Quaternion &objectRotation, const std::string &objectName) const;
        // Given the velocity of an object, calculate the velocity of the object after teleportation
        r3d::Vector3 teleportedVelocity(const r3d::Vector3 &objectVelocity) const;

        // Check if an object has passed through the portal
        float hasPassed(const std::string &objectName) const;
        // Set the surface that the portal is currently on
        void setSurface(Entity *surf);
        // Handle teleportation of an object
        bool passObject(r3d::Collider* objectCollider, const std::string& objectName);
        // Performs calculations and teleports the object
        void teleportObject(const std::string &ObjectName);
        // Constructor to be used by World
        Portal() : Entity() { }

    public:
        // Attributes to be used for teleportation
        // (Cached values)
        Entity* surface = nullptr;
        r3d::Collider* surfaceCollider = nullptr;
        Portal *destination = nullptr;
        glm::fquat portalRot; // glm of localTransform.getRotation()
        glm::fquat invPortalRot; // glm of glm::inverse(portalRot)
        glm::mat4 invLocalToWorld;
        glm::mat4 localToWorld;
        glm::vec4 portalNormal; //(x,y,z,0)
        glm::vec3 portalPosition; //(x,y,z)

        // Once a destination is set then we can calculate
        // the cached values
        void setDestination(Portal* dest) {
            destination = dest;
            // calculateCachedValues();
        }
        // Calculate cached matrices and rotations to be used every teleportation operation
        void calculateCachedValues();
        // RayCast behind portal to get surface
        void getSurface();
        // Adds an object to the list of objects that need to be checked for passing
        bool addToPassing(r3d::Collider* objectCollider, const std::string &objectName);
        // To loop over the objects that need to be checked for passing
        // and check if they passed or not
        // Also remove objects that got teleported
        // Should be called every frame
        void update();
        // Remove an object from the list of objects that need to be checked for passing
        // Adds it to markedForRemoval
        void assertRemoval(const std::string &objectName);

        virtual EntityFactory::EntityType getType() const override { return EntityFactory::EntityType::Portal; }
    };
}