#pragma once

#include "../application.hpp"
#include "../components/free-camera-controller.hpp"
#include "../components/movement.hpp"
#include "../ecs/world.hpp"
#include "../components/RigidBody.hpp"
#include <glm/glm.hpp>
#include "../ecs/player.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <reactphysics3d/reactphysics3d.h>
namespace r3d = reactphysics3d;
namespace portal {

    // The movement system is responsible for moving every entity which contains a MovementComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/movement.hpp"
    class MovementSystem {
    private:
        Player* player = nullptr;
        Application* app = nullptr;
        World *world;
        FreeCameraControllerComponent* controller;
        RigidBodyComponent* playerRigidBody;
        r3d::PhysicsWorld* physicsWorld;
        bool isGrounded = false;
        double lastJumpTime = 0;
        double JumpCoolDown = 0.2;
        std::string attachedName = "";
        Entity* attachement = nullptr;

        // Handles all physics updates
        void physicsUpdate(float deltaTime);
        // Returns velocity component of player
        glm::vec3 handlePlayerMovement();
        // RayCasts and updates isGrounded
        void checkForGround();
        // Handles disattaching and attaching of an entity
        void checkAttachment();

    public:
        MovementSystem(World* world, Application* app) : world(world), app(app) {
            player = dynamic_cast<Player*>(world->getEntityByName("Player"));
            player->setApp(app);
            controller = player->getComponent<FreeCameraControllerComponent>();
            playerRigidBody = player->getComponent<RigidBodyComponent>();
            physicsWorld = player->getWorld()->getPhysicsWorld();
        }

        
        // This should be called every frame to update all entities containing a MovementComponent. 
        void update(World* world, float deltaTime) {
            if(!physicsWorld) return;
            player->update();
            physicsUpdate(deltaTime);
            // For each entity in the world
            for(const auto& [name, entity] : world->getEntities()){
                if(player->getAttachement() && entity == player->getAttachement()) {
                    player->attachToPlayer(entity);
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
                    transform.setPosition(transform.getPosition() - transform.getOrientation() * rgb->relativePosition);
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
