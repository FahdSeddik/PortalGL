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
    class World;
    class RigidBodyComponent;
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
        friend World;

        glm::fquat halfTurn = glm::angleAxis(glm::pi<float>(), glm::vec3(0, 1, 0));
        bool isCalculated = false;
        RigidBodyComponent *objectRgb = nullptr;
        double lastTPtime = 0;
        double teleportationCooldown = 0.2;

        void calculateCachedValues();

        r3d::Vector3 teleportedPosition(const r3d::Vector3 &objectPosition) const;

        r3d::Quaternion teleportedRotation(const r3d::Quaternion &objectRotation, const std::string &objectName) const;

        r3d::Vector3 teleportedVelocity(const r3d::Vector3 &objectVelocity) const;


        bool hasPassed(const std::string &objectName) const;

        bool togObj;
        unsigned short collisionCategory;

        std::unordered_map<std::string, std::shared_ptr<r3d::Collider*>> passedObjects;
        std::unordered_set<std::string> markedForRemoval;

        Portal(): Entity() {}
        void setSurface(Entity *surf);
        bool passObject(r3d::Collider* objectCollider, const std::string& objectName);
    public:
        Entity* surface = nullptr;
        r3d::Collider* surfaceCollider = nullptr;
        Portal *destination = nullptr;
        glm::fquat portalRot; // glm of localTransform.getRotation()
        glm::fquat invPortalRot; // glm of glm::inverse(portalRot)
        glm::mat4 invLocalToWorld;
        glm::mat4 localToWorld;
        glm::vec4 portalNormal; //(x,y,z,0)
        void setDestination(Portal* dest) {
            destination = dest;
            calculateCachedValues();
        }

        void getSurface();
        bool addToPassing(r3d::Collider* objectCollider, const std::string &objectName);

        void update();

        void teleportObject(const std::string &ObjectName);

        void assertRemoval(const std::string &objectName);
    };
}