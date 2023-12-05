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
        bool& isAttached;
        public:
        RayCastInteraction(bool& isAttached) : isAttached(isAttached) {}
        /// Called when a raycast hit a body
        /**
         * @param hit The raycast hit information
         */
        virtual r3d::decimal notifyRaycastHit(const r3d::RaycastInfo& raycastInfo) override {
            // is grounded 
            std::string name = *((std::string*)raycastInfo.body->getUserData());
            std::cout << "RayCast Hit" << name << std::endl;
            if(name == "cube 1")isAttached = true;
            return raycastInfo.hitFraction;
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
        bool& isGrounded;
        bool isAttached = false;
        Entity* attachement = nullptr;
    public:
        MovementSystem(Entity* player, Application* app) : player(player), app(app), isGrounded(player->getWorld()->isGrounded) {
            controller = player->getComponent<FreeCameraControllerComponent>();
            playerRigidBody = player->getComponent<RigidBodyComponent>();
        }



        // This should be called every frame to update all entities containing a MovementComponent. 
        void update(World* world, float deltaTime) {
            r3d::PhysicsWorld* physicsWorld = world->getPhysicsWorld();
            if(!physicsWorld) return;
            // Constant physics time step (60 FPS)
            const float timeStep = 1.0f / 60.0f;
            float delta = deltaTime;
            // Get rigidbody of player 
            glm::mat4 matrix = player->localTransform.toMat4();
            // front: the direction the camera is looking at projected on the xz plane
            // up: global up vector (0,1,0)
            // right: the vector to the right of the camera (x-axis)
            glm::vec3 front = glm::normalize(glm::vec3(matrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
            // project on xz plane
            r3d::Vector3 frontdir(front.x, front.y, front.z);
            frontdir.normalize();
            front.y = 0;
            front = glm::normalize(front);
            //global up
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::vec3(matrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
            // project on xz plane
            right.y = 0;
            right = glm::normalize(right);
            bool jumped = false;
            // Player position
            r3d::Vector3 playerPos = player->localTransform.getPosition();
            // Check if E is pressed
            if(!isAttached && app->getKeyboard().justPressed(GLFW_KEY_E)) {
                // RayCast from player position to front direction with length 1.5
                r3d::Ray ray(playerPos + frontdir,frontdir * 3 + playerPos);
                player->getWorld()->getPhysicsWorld()->raycast(ray, new RayCastInteraction(isAttached));
                if(isAttached) {
                    // Get the entity that is attached
                    for(auto entity : world->getEntities()){
                        if(entity->name == "cube 1") {
                            attachement = entity;
                            break;
                        }
                    }
                }
            } else if (app->getKeyboard().justPressed(GLFW_KEY_E)) {
                attachement->getComponent<RigidBodyComponent>()->getCollider()->setIsTrigger(false);
                attachement = nullptr;
                isAttached = false;
            }

            while(delta > 0.0f){
                // If the remaining time is smaller than the time step, use the remaining time
                float step = glm::min(delta, timeStep);
                glm::vec3 velf = glm::vec3(0.0f, 0.0f, 0.0f);
                glm::vec3 velr = glm::vec3(0.0f, 0.0f, 0.0f);
                // Check if grounded and pressed space then jump
                if(isGrounded && app->getKeyboard().isPressed(GLFW_KEY_SPACE)){
                    // jump Apply force
                    playerRigidBody->getBody()->applyWorldForceAtCenterOfMass(r3d::Vector3(0, controller->positionSensitivity.y * 60.0f, 0));
                    jumped = true;
                }
                if(app->getKeyboard().isPressed(GLFW_KEY_W)){
                    // move forward
                    velf = front * controller->positionSensitivity.z;
                }
                if(app->getKeyboard().isPressed(GLFW_KEY_S)){
                    // move backward
                    velf = -front * controller->positionSensitivity.z;
                }
                if(app->getKeyboard().isPressed(GLFW_KEY_A)){
                    // move left
                    velr = -right * controller->positionSensitivity.x;
                }
                if(app->getKeyboard().isPressed(GLFW_KEY_D)){
                    // move right
                    velr = right * controller->positionSensitivity.x;
                }
                glm::vec3 vel = velf + velr;
                // if shift is pressed, speed up
                if(app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT)){
                    vel *= controller->speedupFactor;
                }
                r3d::Vector3 linearVelocity = playerRigidBody->getBody()->getLinearVelocity();
                playerRigidBody->getBody()->setLinearVelocity(r3d::Vector3(vel.x, linearVelocity.y, vel.z));

                // Update the physics world
                physicsWorld->update(step);
                // Subtract the time step from the remaining time
                delta -= step;
            }
            if(jumped) isGrounded = false;
            // For each entity in the world
            for(auto entity : world->getEntities()){
                if(entity == attachement) {
                    r3d::Transform temp;
                    temp.setPosition(playerPos + frontdir * 3);
                    temp.setOrientation(player->localTransform.getRotation());
                    entity->localTransform.setTransform(temp);
                    RigidBodyComponent* rgb = entity->getComponent<RigidBodyComponent>();
                    rgb->getBody()->setTransform(temp);
                    rgb->getBody()->setLinearVelocity(r3d::Vector3(0,0,0));
                    rgb->getBody()->setAngularVelocity(r3d::Vector3(0,0,0));
                    rgb->getCollider()->setIsTrigger(true);
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
