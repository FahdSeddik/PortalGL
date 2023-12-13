#include "portal.hpp"
#include <reactphysics3d/reactphysics3d.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "world.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>
#include "../components/RigidBody.hpp"
#include "../../states/play-state.hpp"
namespace r3d = reactphysics3d;


namespace portal {

    void Portal::setSurface(Entity *surf) {
        surface = surf;
        surfaceCollider = surface->getComponent<RigidBodyComponent>()->getCollider();
        this->getComponent<RigidBodyComponent>()->getCollider()->setCollideWithMaskBits(1 | 2);
    }

    bool Portal::hasPassed(const std::string& objectName) const {
        Entity* object = getWorld()->getEntityByName(objectName);
        RigidBodyComponent *ObjectRgb = object->getComponent<RigidBodyComponent>();
        glm::vec3 relative_glm(ObjectRgb->relativePosition.x, ObjectRgb->relativePosition.y, ObjectRgb->relativePosition.z);
        // std::cout<<"Relative: "<<relative_glm.x<<" "<<relative_glm.y<<" "<<relative_glm.z<<std::endl;
        glm::vec3 objectPosition = glm::vec3(object->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1)) + relative_glm;
        // std::cout<<"Object: "<<objectPosition.x<<" "<<objectPosition.y<<" "<<objectPosition.z<<std::endl;
        glm::vec3 portalPosition(localToWorld * glm::vec4(0, 0, 0, 1));
        // std::cout<<"Portal: "<<portalPosition.x<<" "<<portalPosition.y<<" "<<portalPosition.z<<std::endl;
        glm::vec4 portalToObj(objectPosition - portalPosition, 0);
        float dot = glm::dot(glm::normalize(portalToObj), portalNormal);
        return dot < 0;
    }
    bool Portal::addToPassing(r3d::Collider* objectCollider, const std::string& objectName) {
        if(passedObjects.find(objectName) != passedObjects.end()) return false;
        if(markedForRemoval.find(objectName) != markedForRemoval.end()) return false;
        passedObjects[objectName] = std::make_shared<r3d::Collider*>(objectCollider);
        return true;
    }

    void Portal::update() {
        if(passedObjects.empty() && markedForRemoval.empty()) {
            surfaceCollider->setIsTrigger(false);
            return;
        }
        for (auto& objectName : markedForRemoval) {
            r3d::Collider *coll = dynamic_cast<r3d::Collider *>(*passedObjects[objectName].get());
            if(!coll->getIsTrigger())coll->setIsTrigger(false);
            passedObjects.erase(objectName);
        }
        markedForRemoval.clear();
        for(auto& [objectName, collider] : passedObjects) {
            passObject(*(collider.get()), objectName);
        }
    }

    bool Portal::passObject(r3d::Collider* objectCollider, const std::string& objectName) {
        if(!destination) return false;
        if(togObj) {
            objectCollider->setIsTrigger(true);
        }
        else {
            surfaceCollider->setIsTrigger(true);
        }
        if (hasPassed(objectName)) {
            teleportObject(objectName);
            if(togObj) {
                objectCollider->setIsTrigger(false);
            }
            else {
                surfaceCollider->setIsTrigger(false);
            }
            assertRemoval(objectName);
            return true;
        }
        return false;
    }

    void Portal::teleportObject(const std::string& ObjectName) {
        if(destination == nullptr) return;
        Entity* object = getWorld()->getEntityByName(ObjectName);
        if(object == surface) return;
        if(object == nullptr) return;
        objectRgb = object->getComponent<RigidBodyComponent>();
        if(objectRgb == nullptr) return;
        if(objectRgb->getBody()->getType() == r3d::BodyType::STATIC) return;
        r3d::Vector3 newPosition = teleportedPosition(object->localTransform.getPosition());
        r3d::Quaternion newRotation = teleportedRotation(object->localTransform.getRotation(), ObjectName);
        r3d::Vector3 newVelocity = teleportedVelocity(objectRgb->getBody()->getLinearVelocity());
        object->localTransform.setTransform(r3d::Transform(newPosition, newRotation));
        objectRgb->getBody()->setTransform(r3d::Transform(newPosition + objectRgb->relativePosition, newRotation));
        objectRgb->getBody()->setLinearVelocity(newVelocity);
    }

    r3d::Vector3 Portal::teleportedVelocity(const r3d::Vector3& objectVelocity) const {
        glm::vec4 objectVelocity_glm(objectVelocity.x, objectVelocity.y, objectVelocity.z, 0);
        glm::vec4 relativeVel = invPortalRot * objectVelocity_glm;
        relativeVel = halfTurn * relativeVel;
        glm::vec4 temp = destination->portalRot * relativeVel;
        return r3d::Vector3(temp.x, temp.y, temp.z);
    }

    r3d::Quaternion Portal::teleportedRotation(const r3d::Quaternion& objectRotation, const std::string& objectName) const {
        glm::fquat objectRotation_glm(objectRotation.w, objectRotation.x, objectRotation.y, objectRotation.z);
        glm::fquat relativeRot = invPortalRot * objectRotation_glm;
        relativeRot = halfTurn * relativeRot;
        glm::fquat newObjectRotation_glm = destination->portalRot * relativeRot;
        newObjectRotation_glm = glm::normalize(newObjectRotation_glm);
        if (objectName == "Player" && ((std::abs(portalNormal.y) > 0.7f) ^ (std::abs(destination->portalNormal.y) > 0.7f))) {
            // get right
            // get up of other portal
            glm::vec3 forced_up(0, 1, 0);
            glm::vec3 forced_front = newObjectRotation_glm * glm::vec3(0, 0, 1);
            forced_front = glm::normalize(forced_front);
            glm::vec3 right = glm::cross(forced_up, forced_front);
            forced_front = glm::cross(right, forced_up);
            right = glm::normalize(glm::cross(forced_up, forced_front));
            // Quaternion.LockRotation((0, 1, 0), right);
            glm::mat4 newRotation = glm::mat4(1.0f);
            newRotation[0][0] = right.x;
            newRotation[1][0] = right.y;
            newRotation[2][0] = right.z;
            newRotation[0][1] = forced_up.x;
            newRotation[1][1] = forced_up.y;
            newRotation[2][1] = forced_up.z;
            newRotation[0][2] = forced_front.x;
            newRotation[1][2] = forced_front.y;
            newRotation[2][2] = forced_front.z;
            newObjectRotation_glm = glm::quat_cast(newRotation);
        }
        newObjectRotation_glm = glm::normalize(newObjectRotation_glm);
        return r3d::Quaternion(newObjectRotation_glm.x, newObjectRotation_glm.y, newObjectRotation_glm.z, newObjectRotation_glm.w);
    }

    r3d::Vector3 Portal::teleportedPosition(const r3d::Vector3& objectPosition) const {
        glm::vec4 objectPosition_glm(objectPosition.x, objectPosition.y, objectPosition.z, 1);
        glm::vec4 relativePos = invLocalToWorld * objectPosition_glm;
        relativePos = halfTurn * relativePos;
        glm::vec4 temp = destination->localToWorld * relativePos;
        if(std::abs(portalNormal.y) > 0.7f || std::abs(destination->portalNormal.y) > 0.7f) {
            // get bounds of object
            r3d::Vector3 min, max;
            objectRgb->getCollider()->getCollisionShape()->getLocalBounds(min, max);
            // get height
            float height = max.y - min.y;
            temp += destination->portalNormal * height;
        } else temp += destination->portalNormal * 0.5f;
        return r3d::Vector3(temp.x, temp.y, temp.z);
    }

    void Portal::calculateCachedValues() {
        const r3d::Quaternion& rot = localTransform.getRotation();
        portalRot = glm::fquat(rot.w, rot.x, rot.y, rot.z);
        invPortalRot = glm::inverse(portalRot);
        localToWorld = getLocalToWorldMatrix();
        invLocalToWorld = glm::inverse(localToWorld);
        portalNormal = glm::vec4(portalRot * glm::vec3(0, 0, 1), 0);
        togObj = std::abs(portalNormal.y) > 0.7f;
    }
    void Portal::getSurface() {
        std::string surfaceName = "";
        RayCastGetSurface rayCastHandler(surfaceName);
        // Ray Cast from center of player to bottom of player
        glm::vec4 start(localToWorld * glm::vec4(0, 0, 0, 1));
        glm::vec4 end = start - portalNormal;
        r3d::Ray ray(r3d::Vector3(start.x, start.y, start.z), r3d::Vector3(end.x, end.y, end.z));
        getWorld()->getPhysicsWorld()->raycast(ray, &rayCastHandler);
        if(surfaceName != "") {
            Entity* surface = getWorld()->getEntityByName(surfaceName);
            if(surface != nullptr) {
                setSurface(surface);
            }
        }
    }
    void Portal::assertRemoval(const std::string &objectName) {
        if(passedObjects.count(objectName)) {
            markedForRemoval.insert(objectName);
        }
    }
}