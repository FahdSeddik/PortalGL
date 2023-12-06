#include "movement.hpp"

namespace portal {
    void MovementSystem::attachToPlayer(Entity* entity) const {
        r3d::Transform temp;
        // Set the position of the entity to be in front of the player
        temp.setPosition(playerPos + absoluteFront * 3);
        // Set the orientation of the entity to be the same as it was picked up
        temp.setOrientation(entity->localTransform.getRotation());
        entity->localTransform.setTransform(temp);
        RigidBodyComponent* rgb = entity->getComponent<RigidBodyComponent>();
        // Make sure rigidbody is synced with the transform of the entity
        rgb->getBody()->setTransform(temp);
        rgb->getBody()->setLinearVelocity(r3d::Vector3(0,0,0));
        rgb->getBody()->setAngularVelocity(r3d::Vector3(0,0,0));
        // Make sure the collider is a trigger to avoid false collisions
        rgb->getCollider()->setIsTrigger(true);
    }

    void MovementSystem::physicsUpdate(float deltaTime) {
        // Constant physics time step (60 FPS)
        const float timeStep = 1.0f / 60.0f;
        float delta = deltaTime;
        bool jumped = false;
        while(delta > 0.0f){
            // If the remaining time is smaller than the time step, use the remaining time
            glm::vec3 vel = handlePlayerMovement(jumped);
            r3d::Vector3 linearVelocity = playerRigidBody->getBody()->getLinearVelocity();
            playerRigidBody->getBody()->setLinearVelocity(r3d::Vector3(vel.x, linearVelocity.y, vel.z));
            float step = glm::min(delta, timeStep);
            // Update the physics world
            physicsWorld->update(step);
            // Subtract the time step from the remaining time
            delta -= step;
        }
        if(jumped) isGrounded = false;
    }

    glm::vec3 MovementSystem::handlePlayerMovement(bool& jumped) {
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
            velf += front * controller->positionSensitivity.z;
        }
        if(app->getKeyboard().isPressed(GLFW_KEY_S)){
            // move backward
            velf += -front * controller->positionSensitivity.z;
        }
        if(app->getKeyboard().isPressed(GLFW_KEY_A)){
            // move left
            velr += -right * controller->positionSensitivity.x;
        }
        if(app->getKeyboard().isPressed(GLFW_KEY_D)){
            // move right
            velr += right * controller->positionSensitivity.x;
        }
        glm::vec3 vel = velf + velr;
        // if shift is pressed, speed up
        if(app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT)){
            vel *= controller->speedupFactor;
        }
        vel.y = 0;
        return vel;
    }


    void MovementSystem::calculatePlayerVectors() {
        // Get rigidbody of player 
        glm::mat4 matrix = player->localTransform.toMat4();
        // front: the direction the camera is looking at projected on the xz plane
        // up: global up vector (0,1,0)
        // right: the vector to the right of the camera (x-axis)
        front = glm::normalize(glm::vec3(matrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
        absoluteFront = r3d::Vector3(front.x, front.y, front.z);
        absoluteFront.normalize();
        // project on xz plane
        front.y = 0;
        front = glm::normalize(front);
        //global up
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        right = glm::normalize(glm::vec3(matrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
        // project on xz plane
        right.y = 0;
        right = glm::normalize(right);
    }

    void MovementSystem::checkAttachment() {
        // Check if E is pressed
        if(!attachement && app->getKeyboard().justPressed(GLFW_KEY_E)) {
            // RayCast from player position to front direction with length 1.5
            r3d::Ray ray(playerPos + absoluteFront,absoluteFront * 3 + playerPos);
            physicsWorld->raycast(ray, new RayCastInteraction(attachedName));
            if(attachedName.empty()) return;
            // get entity with current attachedName and check if it is attachable
            Entity *potentialAttachement = player->getWorld()->getEntityByName(attachedName);
            if(potentialAttachement->isAttachable) {
                attachement = potentialAttachement;
            }
        } else if (app->getKeyboard().justPressed(GLFW_KEY_E)) {
            // If E is pressed and we have an attachement then we detach it
            // attachment should return to not be a trigger to collide with other objects
            attachement->getComponent<RigidBodyComponent>()->getCollider()->setIsTrigger(false);
            attachement = nullptr;
            attachedName = "";
        }
    }

}