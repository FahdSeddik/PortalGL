#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"
#include "../components/RigidBody.hpp"
#include "../components/free-camera-controller.hpp"
#include "../ecs/portal.hpp"
#include "../application.hpp"
#include <reactphysics3d/reactphysics3d.h>
namespace r3d = reactphysics3d;
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace portal {

    // The movement system is responsible for moving every entity which contains a MovementComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/movement.hpp"
    class MovementSystem {
    private:
        Entity* player = nullptr;
        Application *app;
        World *world;
        FreeCameraControllerComponent* controller;
        RigidBodyComponent* playerRigidBody;
        r3d::PhysicsWorld* physicsWorld;
        bool isGrounded = false;
        double lastJumpTime = 0;
        double JumpCoolDown = 0.2;
        std::string attachedName = "";
        Entity* attachement = nullptr;
        // Hold portals 
        // TODO: this needs to be dynamic 
        // for shooting portals
        Portal* Portal_1 = nullptr;
        Portal* Portal_2 = nullptr;
        bool isPortal1Shot = false;
        bool isPortal2Shot = false;

        // Player Vectors
        r3d::Vector3 absoluteFront;
        glm::vec3 front;
        glm::vec3 right;
        // Player position reference
        const r3d::Vector3 &playerPos;
        
        // Attach an "isAttachable" entity to the player
        void attachToPlayer(Entity *entity) const;
        // Handles all physics updates
        void physicsUpdate(float deltaTime);

        // Returns velocity component of player
        glm::vec3 handlePlayerMovement();
        // gets the front, right, and absoluteFront of player
        void calculatePlayerVectors();
        // RayCasts and updates isGrounded
        void checkForGround();
        // Handles disattaching and attaching of an entity
        void checkAttachment();
    public:
        MovementSystem(World* world, Application* app) : world(world), app(app), player(world->getEntityByName("Player")), playerPos(player->localTransform.getPosition()) {
            controller = player->getComponent<FreeCameraControllerComponent>();
            playerRigidBody = player->getComponent<RigidBodyComponent>();
            physicsWorld = player->getWorld()->getPhysicsWorld();
            Portal_1 = dynamic_cast<Portal*>(world->getEntityByName("Portal_1"));
            Portal_2 = dynamic_cast<Portal*>(world->getEntityByName("Portal_2"));
            // Portal_1->setDestination(Portal_2);
            // Portal_2->setDestination(Portal_1);
            // Portal_1->getSurface();
            // Portal_2->getSurface();
        }

        
        // This should be called every frame to update all entities containing a MovementComponent. 
        void update(World* world, float deltaTime) {
            if(!physicsWorld) return;
            Portal_1->update();
            Portal_2->update();
            calculatePlayerVectors();
            checkAttachment();
            // checkPortalShot();
            physicsUpdate(deltaTime);
            // For each entity in the world
            for(const auto& [name, entity] : world->getEntities()){
                if(attachement && entity == attachement) {
                    attachToPlayer(entity);
                    continue;
                }
                // Get the movement component if it exists
                MovementComponent* movement = entity->getComponent<MovementComponent>();
                // If the movement component exists
                if(movement){
                    // Change the position and rotation based on the linear & angular velocity and delta time.
                    const r3d::Vector3& pos = entity->localTransform.getPosition();
                    const r3d::Quaternion& rot = entity->localTransform.getRotation();
                    glm::vec3 position(pos.x, pos.y, pos.z);
                    glm::quat rotation((float)rot.w, (float)rot.x, (float)rot.y, (float)rot.z);
                    position += deltaTime * movement->linearVelocity;
                    rotation = rotation + 0.5f * deltaTime * movement->angularVelocity * rotation;
                    rotation = glm::normalize(rotation);
                    entity->localTransform.setPosition(position);
                    entity->localTransform.setRotation(rotation);
                }
                RigidBodyComponent* rgb = entity->getComponent<RigidBodyComponent>();
                if(rgb){
                    if(rgb->getBody()->getType() == r3d::BodyType::STATIC) continue;
                    FreeCameraControllerComponent* fcc = entity->getComponent<FreeCameraControllerComponent>();
                    r3d::Transform transform = rgb->getBody()->getTransform();
                    transform.setPosition(transform.getPosition() - rgb->relativePosition);
                    if(fcc){
                        // orientation stays the same
                        transform.setOrientation(entity->localTransform.getRotation());
                        entity->localTransform.setTransform(transform);
                        rgb->getBody()->setAngularVelocity(r3d::Vector3(0,0,0));
                    }else{
                        entity->localTransform.setTransform(transform);
                    }
                }
            }
        }

    };

}
