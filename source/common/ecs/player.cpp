#include "player.hpp"
#include "../components/RigidBody.hpp"
#include <GLFW/glfw3.h>
#include "../application.hpp"
#include "world.hpp"
namespace portal {
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

    void Player::update() {
        // Calculate player vectors
        calculatePlayerVectors();
        // Check if player is attached to an entity
        checkAttachment();
    }

    void Player::attachToPlayer(Entity* entity) const {
        r3d::Transform temp;
        // Set the position of the entity to be in front of the player
        temp.setPosition(localTransform.getPosition() + absoluteFront * 3);
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
    
    void Player::checkAttachment() {
        // Check if E is pressed
        if(!attachement && app->getKeyboard().justPressed(GLFW_KEY_E)) {
            // RayCast from player position to front direction with length 1.5
            r3d::Ray ray(localTransform.getPosition() + absoluteFront,absoluteFront * 3 + localTransform.getPosition());
            getWorld()->getPhysicsWorld()->raycast(ray, new RayCastInteraction(attachementName));
            if(attachementName.empty()) return;
            // get entity with current attachementName and check if it is attachable
            Entity *potentialAttachement = getWorld()->getEntityByName(attachementName);
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
            attachementName = "";
        }
    }

    void Player::calculatePlayerVectors() {
        // Get Matrix of player
        glm::mat4 matrix = getLocalToWorldMatrix();
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
}