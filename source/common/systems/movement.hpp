#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"
#include "../components/RigidBody.hpp"
#include "../components/free-camera-controller.hpp"

#include "../application.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace portal
{

    // The movement system is responsible for moving every entity which contains a MovementComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/movement.hpp"
    class MovementSystem {
    private:
        Entity* player = nullptr;
        Application *app;

    public:
        void init(Entity* player, Application* app) {
            this->player = player;
            this->app = app;
        }



        // This should be called every frame to update all entities containing a MovementComponent. 
        void update(World* world, float deltaTime) {
            r3d::PhysicsWorld* physicsWorld = world->getPhysicsWorld();
            if(!physicsWorld) return;
            // Constant physics time step (60 FPS)
            const float timeStep = 1.0f / 60.0f;
            float delta = deltaTime;
            // Get rigidbody of player 
            RigidBodyComponent* playerRigidBody = player->getComponent<RigidBodyComponent>();
            glm::mat4 matrix = player->localTransform.toMat4();

            // front: the direction the camera is looking at projected on the xz plane
            // up: global up vector (0,1,0)
            // right: the vector to the right of the camera (x-axis)
            glm::vec3 front = glm::normalize(glm::vec3(matrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
            // project on xz plane
            front.y = 0;
            front = glm::normalize(front);
            //global up
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::vec3(matrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
            // project on xz plane
            right.y = 0;
            right = glm::normalize(right);
            FreeCameraControllerComponent* controller = player->getComponent<FreeCameraControllerComponent>();
            while(delta > 0.0f){
                // If the remaining time is smaller than the time step, use the remaining time
                float step = glm::min(delta, timeStep);
                glm::vec3 velf = glm::vec3(0.0f, 0.0f, 0.0f);
                glm::vec3 velr = glm::vec3(0.0f, 0.0f, 0.0f);
                r3d::Vector3 linearVelocity = playerRigidBody->getBody()->getLinearVelocity();
                // if shift is pressed, speed up

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
                playerRigidBody->getBody()->setLinearVelocity(r3d::Vector3(vel.x, linearVelocity.y, vel.z));

                // if linear velocity exceeds max speed, set it to max speed
                // if(linearVelocity.x > 3){
                //     linearVelocity.x = 3;
                // }
                // if(linearVelocity.z > 3){
                //     linearVelocity.z = 3;
                // }
                // playerRigidBody->getBody()->setLinearVelocity(linearVelocity);

                // Update the physics world
                physicsWorld->update(step);
                // Subtract the time step from the remaining time
                delta -= step;
            }
            // For each entity in the world
            for(auto entity : world->getEntities()){
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
                    FreeCameraControllerComponent* fcc = entity->getComponent<FreeCameraControllerComponent>();
                    r3d::Transform transform = rgb->getBody()->getTransform();
                    transform.setPosition(transform.getPosition() - rgb->relativePosition);
                    if(fcc){
                        // orientation stays the same
                        transform.setOrientation(entity->localTransform.getRotation());
                        entity->localTransform.setTransform(transform);
                        // This disables the player to get influenced by other objects in a way that it
                        // glitches and flies off the map
                        // r3d::Vector3 linearVelocity = rgb->getBody()->getLinearVelocity();
                        // linearVelocity.x = linearVelocity.z = 0;
                        // rgb->getBody()->setLinearVelocity(linearVelocity);
                        rgb->getBody()->setAngularVelocity(r3d::Vector3(0,0,0));
                    }else{
                        entity->localTransform.setTransform(transform);
                    }
                }
            }
        }

    };

}
