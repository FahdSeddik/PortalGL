#include "movement.hpp"

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

    // Class Handle portal shooting
    class RayCastPortal : public r3d::RaycastCallback {
        std::string& surfaceName;
        glm::vec3& hitPoint;
        public:
        RayCastPortal(std::string& surfaceName, glm::vec3& hitPoint) : surfaceName(surfaceName) , hitPoint(hitPoint){}
        // Called when a raycast hits a body
        virtual r3d::decimal notifyRaycastHit(const r3d::RaycastInfo& raycastInfo) override {
            // Get the name of the body that has been hit
            if(raycastInfo.body->getCollider(0)->getIsTrigger()){
                // if trigger, return 1.0 to continue raycast
                return r3d::decimal(1.0);
            }
            surfaceName = *((std::string*)raycastInfo.body->getUserData());
            hitPoint = glm::vec3(raycastInfo.worldPoint.x, raycastInfo.worldPoint.y, raycastInfo.worldPoint.z);
            std::cout << "RayCast Hit" << surfaceName << " at " << hitPoint.x << " " << hitPoint.y << " " << hitPoint.z << std::endl;
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

    void MovementSystem::castPortal(Entity* surface, Portal* portal, glm::vec3 hitPoint) {
        // place portal on the surface with correct orientation
        
        // step 1 set the position of the portal to the position of the surface
        // get the surface location and normal
        r3d::Vector3 surfaceLocation = surface->localTransform.getPosition();
        r3d::Vector3 surfaceNormal = surface->localTransform.getRotation() * r3d::Vector3(0,0,1);
        glm::vec3 surfaceLocationGlm = {surfaceLocation.x, surfaceLocation.y, surfaceLocation.z};
        glm::vec3 surfaceNormalGlm = {surfaceNormal.x, surfaceNormal.y, surfaceNormal.z};

        // step 2 set the orientation of the portal correctly
        // to set the orientation we need to know the up vector of the portal and its normal
        // the normal is the same as the surface normal
        // to set the up vector we need to check if the surface is a floor, ceiling, wall
        // if it is a floor or ceiling we need to project the player position on the surface 
        // then calculate the vector from the raycast hit to the player projected on the surface
        // that vector is the down vector of the portal and its negative is the up vector
        // and the front vector is the normal of the surface
        // if it is a wall then the up vector is the global up vector and the front vector is the normal of the surface
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        if(std::abs(glm::dot(surfaceNormalGlm, glm::vec3(0,1,0))) > 0.9) {
            // if the surface is a floor or ceiling
            // get the player position
            glm::vec3 playerPosition = {playerPos.x, playerPos.y, playerPos.z};
            // get the vector from the raycast hit to the player
            glm::vec3 surfaceToPlayer = playerPosition - hitPoint;
            // project the player position on the surface
            glm::vec3 projectedPlayerPosition = playerPosition - glm::dot(surfaceToPlayer, surfaceNormalGlm) * surfaceNormalGlm;
            // get the down vector of the portal
            glm::vec3 down = projectedPlayerPosition - hitPoint;
            // get the up vector of the portal
            up =  glm::normalize(-down);
                                
        } 
        glm::vec3 front = surfaceNormalGlm;
        glm::vec3 right = glm::normalize(glm::cross(up, front));
        // front = glm::normalize(glm::cross(right, up));
        // right = glm::normalize(glm::cross(up, front));
        glm::mat4 orientation = glm::mat4(glm::vec4(right, 0.0f), glm::vec4(up, 0.0f), glm::vec4(front, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        
        glm::quat orientationQuat = glm::quat_cast(orientation);
        orientationQuat = glm::normalize(orientationQuat);
        r3d::Quaternion orientationR3d = r3d::Quaternion(orientationQuat.x, orientationQuat.y, orientationQuat.z, orientationQuat.w);
        r3d::Vector3 pos = r3d::Vector3(hitPoint.x, hitPoint.y, hitPoint.z);
        pos += surfaceNormal * 0.1f;
        // set the orientation of the portal
        portal->localTransform.setTransform(r3d::Transform(pos, orientationR3d));
        portal->getComponent<RigidBodyComponent>()->getBody()->setTransform(r3d::Transform(pos + Portal_1->getComponent<RigidBodyComponent>()->relativePosition, orientationR3d));
        
        // calculate the cached values of the portal (e.g. the localToWorld matrix)
        portal->calculateCachedValues();
        // gets the surface behind the portal and sets the surface data memeber
        portal->getSurface();
    }

    void MovementSystem::checkPortalShot(){
         // Check if Mouse left is pressed
        if(app->getMouse().justPressed(GLFW_MOUSE_BUTTON_1)) {
            std::string name = "";
            glm::vec3 hitPoint = glm::vec3(0.0f, 0.0f, 0.0f);
            // RayCast from player position to front direction with length 50
            r3d::Ray ray(playerPos + absoluteFront,absoluteFront * 50 + playerPos);
            physicsWorld->raycast(ray, new RayCastPortal(name, hitPoint));
            if(name.empty()) return;
            // get entity with current name and check if it is can hold a portal
            Entity *potentialPortalSurface = player->getWorld()->getEntityByName(name);
            if(potentialPortalSurface->canHoldPortal) {
                // if portal can be placed, place it
                castPortal(potentialPortalSurface, Portal_1, hitPoint);
                
                isPortal1Shot = true;

                if(isPortal2Shot){
                    Portal_1->setDestination(Portal_2);
                    Portal_2->setDestination(Portal_1);
                }
            }

        } else if (app->getMouse().justPressed(GLFW_MOUSE_BUTTON_2)) {
            std::string name = "";
            glm::vec3 hitPoint = glm::vec3(0.0f, 0.0f, 0.0f);
            // RayCast from player position to front direction with length 50
            r3d::Ray ray(playerPos + absoluteFront, absoluteFront * 50 + playerPos);
            physicsWorld->raycast(ray, new RayCastPortal(name, hitPoint));
            if(name.empty()) return;
            // get entity with current name and check if it can hold a portal
            Entity *potentialPortalSurface = player->getWorld()->getEntityByName(name);
            
            // if portal can be placed, place it
            if(potentialPortalSurface->canHoldPortal) {
                castPortal(potentialPortalSurface, Portal_2, hitPoint);

                isPortal2Shot = true;

                // if both portals are shot, set the destination of each portal to the other
                if(isPortal1Shot){
                    Portal_1->setDestination(Portal_2);
                    Portal_2->setDestination(Portal_1);
                }
            }
        }
    }
}