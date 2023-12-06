#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"
#include "../components/RigidBody.hpp"
#include "../components/free-camera-controller.hpp"

#include "../application.hpp"
#include <reactphysics3d/reactphysics3d.h>
namespace r3d = reactphysics3d;
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace portal
{

    // Class Handle player Grounded
    class RayCastInteraction : public r3d::RaycastCallback {
        std::string& attachedName;
        public:
        RayCastInteraction(std::string& attachedName) : attachedName(attachedName) {}
        // Called when a raycast hits a body
        virtual r3d::decimal notifyRaycastHit(const r3d::RaycastInfo& raycastInfo) override {
            // Get the name of the body that has been hit
            if(raycastInfo.body->getCollider(0)->getIsTrigger()){
                // if trigger, return 1.0 to continue raycast
                return r3d::decimal(1.0);
            }
            attachedName = *((std::string*)raycastInfo.body->getUserData());
            std::cout << "RayCast Hit" << attachedName << std::endl;
            // return 0 to stop raycast
            return r3d::decimal(0.0);
        }
    };


    // The movement system is responsible for moving every entity which contains a MovementComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/movement.hpp"
    class MovementSystem {
    private:
        Entity* player = nullptr;
        Application *app;
        FreeCameraControllerComponent* controller;
        RigidBodyComponent* playerRigidBody;
        r3d::PhysicsWorld* physicsWorld;
        bool& isGrounded;
        std::string attachedName = "";
        Entity* attachement = nullptr;

        // Player Vectors
        r3d::Vector3 absoluteFront;
        glm::vec3 front;
        glm::vec3 right;
        // Player position reference
        const r3d::Vector3 &playerPos;

        void attachToPlayer(Entity *entity) const;

        void physicsUpdate(float deltaTime);

        // Returns velocity component of player
        glm::vec3 handlePlayerMovement(bool &jumped);

        void calculatePlayerVectors();

        void checkAttachment();

    public:
        MovementSystem(Entity* player, Application* app) : player(player), app(app), isGrounded(player->getWorld()->isGrounded), playerPos(player->localTransform.getPosition()) {
            controller = player->getComponent<FreeCameraControllerComponent>();
            playerRigidBody = player->getComponent<RigidBodyComponent>();
            physicsWorld = player->getWorld()->getPhysicsWorld();
        }

        
        // This should be called every frame to update all entities containing a MovementComponent. 
        void update(World* world, float deltaTime) {
            if(!physicsWorld) return;
            calculatePlayerVectors();
            checkAttachment();
            physicsUpdate(deltaTime);
            // For each entity in the world
            for(const auto& [name, entity] : world->getEntities()){
                if(entity == attachement) {
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
