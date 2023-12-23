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
        const glm::vec3& right = player->getRight();
        const glm::vec3& front = player->getFront();
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
}