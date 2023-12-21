#include "movement.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace portal {

    // Class Handle player Grounded
    class RayCastPlayerGrounded : public r3d::RaycastCallback {
        bool& isGrounded;
        public:
        RayCastPlayerGrounded(bool& isGrounded) : isGrounded(isGrounded) {}
        /// Called when a raycast hit a body
        /**
         * @param hit The raycast hit information
         */
        virtual r3d::decimal notifyRaycastHit(const r3d::RaycastInfo& raycastInfo) override {
            // is grounded 
            // std::cout << "Player Grounded: " << *((std::string*)raycastInfo.body->getUserData()) << std::endl;
            if(raycastInfo.body->getCollider(0)->getCollideWithMaskBits() != 0x0001 || raycastInfo.body->getCollider(0)->getIsTrigger()){
                // if trigger, return 1.0 to continue raycast
                return r3d::decimal(1.0);
            }
            isGrounded = true;
            // return 0 to stop raycast
            return r3d::decimal(0.0);
        }
    };

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

    
    void MovementSystem::checkForGround() {
        RayCastPlayerGrounded rayCastHandler(isGrounded);
        // get player position and bounds
        r3d::Vector3 pos, min, max;
        pos = playerRigidBody->getBody()->getTransform().getPosition();
        playerRigidBody->getCollider()->getCollisionShape()->getLocalBounds(min, max);
        // RayCast to check if player is grounded
        r3d::decimal y = max.y - min.y;
        // Ray Cast from center of player to bottom of player
        r3d::Ray ray(pos, pos - r3d::Vector3(0, y/2, 0));
        physicsWorld->raycast(ray, &rayCastHandler);
    }

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
        // transfer collider to separate layer to disable it from colliding with other objects
        rgb->getCollider()->setCollideWithMaskBits(0);
        rgb->getCollider()->setCollisionCategoryBits(0);
    }

    void MovementSystem::physicsUpdate(float deltaTime) {
        if(!playerRigidBody->getCollider()->getIsTrigger() && glfwGetTime() - lastJumpTime > JumpCoolDown && app->getKeyboard().isPressed(GLFW_KEY_SPACE)) {
            // if space pressed make sure to update isGrounded
            checkForGround();
        }
        // Constant physics time step (60 FPS)
        const float timeStep = 1.0f / 60.0f;
        float delta = deltaTime;
        while(delta > 0.0f){
            // If the remaining time is smaller than the time step, use the remaining time
            glm::vec3 vel = handlePlayerMovement();
            r3d::Vector3 linearVelocity = playerRigidBody->getBody()->getLinearVelocity();
            playerRigidBody->getBody()->setLinearVelocity(r3d::Vector3(vel.x, linearVelocity.y, vel.z));
            float step = glm::min(delta, timeStep);
            // Update the physics world
            physicsWorld->update(step);
            // Subtract the time step from the remaining time
            delta -= step;
        }
    }

    glm::vec3 MovementSystem::handlePlayerMovement() {
        glm::vec3 velf = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 velr = glm::vec3(0.0f, 0.0f, 0.0f);
        // Check if grounded and pressed space then jump
        if(isGrounded && app->getKeyboard().isPressed(GLFW_KEY_SPACE)){
            // jump Apply force
            playerRigidBody->getBody()->applyWorldForceAtCenterOfMass(r3d::Vector3(0, controller->positionSensitivity.y * 60.0f, 0));
            isGrounded = false;
            lastJumpTime = glfwGetTime();
        }
        if(app->getKeyboard().isPressed(GLFW_KEY_W)){
            // move forward
            velf += front;
        }
        if(app->getKeyboard().isPressed(GLFW_KEY_S)){
            // move backward
            velf += -front;
        }
        if(app->getKeyboard().isPressed(GLFW_KEY_A)){
            // move left
            velr += -right;
        }
        if(app->getKeyboard().isPressed(GLFW_KEY_D)){
            // move right
            velr += right;
        }
        glm::vec3 vel = velf + velr;
        if(vel == glm::vec3(0.0f, 0.0f, 0.0f)) return vel;
        vel = glm::normalize(vel);
        vel *= controller->positionSensitivity;
        // if shift is pressed, speed up
        if(app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT)){
            vel *= controller->speedupFactor;
        }
        vel.y = 0;
        return vel;
    }


    void MovementSystem::calculatePlayerVectors() {
        // Get Matrix of player
        glm::mat4 matrix = player->getLocalToWorldMatrix();
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
            r3d::Collider *collider = attachement->getComponent<RigidBodyComponent>()->getCollider();
            // return collider to same layer as other objects to collide with them again
            collider->setCollideWithMaskBits(1);
            collider->setCollisionCategoryBits(1);
            attachement = nullptr;
            attachedName = "";
        }
    }


}