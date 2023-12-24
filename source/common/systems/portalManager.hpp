#pragma once

#include "../ecs/world.hpp"
#include "../components/RigidBody.hpp"
#include "../ecs/portal.hpp"
#include "../application.hpp"
#include "../ecs/player.hpp"
#include <reactphysics3d/reactphysics3d.h>
namespace r3d = reactphysics3d;


namespace portal {
    class PortalManager {
    private:
        // Class Handle portal shooting
        class RayCastPortal : public r3d::RaycastCallback {
            std::string& surfaceName;
            glm::vec3& hitPoint;
            float distance = FLT_MAX;
            public:
            RayCastPortal(std::string& surfaceName, glm::vec3& hitPoint) : surfaceName(surfaceName) , hitPoint(hitPoint){}
            // Called when a raycast hits a body
            virtual r3d::decimal notifyRaycastHit(const r3d::RaycastInfo& raycastInfo) override {
                // Get the name of the body that has been hit
                if(raycastInfo.body->getCollider(0)->getIsTrigger()){
                    // if trigger, return 1.0 to continue raycast
                    return r3d::decimal(1.0);
                }
                if(raycastInfo.hitFraction < distance){
                    distance = raycastInfo.hitFraction;
                    // update hit point and surface name
                    surfaceName = *((std::string*)raycastInfo.body->getUserData());
                    hitPoint = glm::vec3(raycastInfo.worldPoint.x, raycastInfo.worldPoint.y, raycastInfo.worldPoint.z);
                    // std::cout << "RayCast Hit" << surfaceName << " at " << hitPoint.x << " " << hitPoint.y << " " << hitPoint.z << std::endl;
                }
                return r3d::decimal(1.0);

            }
        };

        struct Rectangle {
            glm::vec2 topLeft;
            glm::vec2 topRight;
            glm::vec2 bottomLeft;
            glm::vec2 bottomRight;
            glm::vec2 center;
        }; 

        enum class Point {
            TOP_LEFT,
            TOP_RIGHT,
            BOTTOM_LEFT,
            BOTTOM_RIGHT
        };

        Player* player = nullptr;
        Application *app;
        World *world;

        r3d::PhysicsWorld* physicsWorld;

        // Hold portals 
        Portal* Portal_1 = nullptr;
        Portal* Portal_2 = nullptr;
        bool isPortal1Shot = false;
        bool isPortal2Shot = false;
        float portalMaxDistance = 100.0f;
        
        // Validates and casts portals
        void checkPortalShot();
        // Casts a portal
        bool castPortal(Entity* surface, Portal* portal, glm::vec3 hitPoint);
        
        // Calculates the correction vector for the portal
        glm::vec2 calculateSurfaceCorrectionVector(glm::vec2 surfaceMin, glm::vec2 surfaceMax, glm::vec2 portalMin, glm::vec2 portalMax);

        // Calculates the intersection of two lines
        glm::vec2 lineLineIntersection(glm::vec2 A, glm::vec2 B, glm::vec2 C, glm::vec2 D);

        // Checks if a point is inside a rectangle
        bool isPointInsideRectangle(const glm::vec2& point, const Rectangle& rectangle);

        // Calculates the overlap correction vector for two portals
        glm::vec2 calculatePortalOverlapCorrectionVector(Rectangle& portal, Rectangle& otherPortal);

        // Calculates the overlap correction vector for two portals
        // the corrected position is returned in hitPoint (by reference)
        bool getCorrectedPortalPos(Entity* surface, Portal* otherPortal, glm::vec3& hitPoint, glm::vec3 up, glm::vec3 right, glm::vec3 front);
        
        // get the 2 intersection points of a line and a rectangle
        // the points are returned in an array of size 2
        std::vector<glm::vec2> getLineRectangleIntersectionPoints(Rectangle& rectanglePortal, Rectangle& rectangleOtherPortal, Point point);
    public:
        PortalManager(World* world, Application* app) : world(world), app(app){
            player = dynamic_cast<Player*>(world->getEntityByName("Player"));
            this->physicsWorld = world->getPhysicsWorld();
            Portal_1 = dynamic_cast<Portal*>(world->getEntityByName("Portal_1"));
            Portal_2 = dynamic_cast<Portal*>(world->getEntityByName("Portal_2"));
        }


        // Handles portal shooting
        void update(){
            Portal_1->update();
            Portal_2->update();
            checkPortalShot();
        }

    };
}
