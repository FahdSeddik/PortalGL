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
        this->getComponent<RigidBodyComponent>()->getCollider()->setCollideWithMaskBits(1);
    }

    void Portal::calculateFailSafeLocation(const std::string& objectName) {
        Entity* object = getWorld()->getEntityByName(objectName);
        if(object == nullptr) return;
        // Object position from model matrix of object + the relative position of the collider
        glm::vec3 objectPosition = glm::vec3(object->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1));
        // vector from portal center to object center
        glm::vec4 portalToObj(objectPosition - portalPosition, 0);
        // project vector onto plane of portal
        glm::vec4 projected = portalToObj - glm::dot(portalToObj, portalNormal) * portalNormal;
        // add projected vector to portal position to get failSafeTeleportLocation
        glm::vec4 failSafeTeleportLocation_glm = glm::vec4(portalPosition, 1) + projected;
        failSafeTeleportLocation[objectName] = failSafeTeleportLocation_glm;
    }

    bool Portal::shouldUseFailSafeLocation(const r3d::Vector3 &objectPosition) const {
        // check length of vector from portal to object 
        glm::vec4 portalToObj(objectPosition.x - portalPosition.x, objectPosition.y - portalPosition.y, objectPosition.z - portalPosition.z, 0);
        float length = glm::length(portalToObj);
        return length > 2.0f;
    }


    float Portal::hasPassed(const std::string& objectName) const {
        // Get entity
        // Calculate dot product between portal normal and vector from portal to object
        // if dot product is negative then object has passed through portal
        Entity* object = getWorld()->getEntityByName(objectName);
        RigidBodyComponent *ObjectRgb = object->getComponent<RigidBodyComponent>();
        glm::vec3 relative_glm(ObjectRgb->relativePosition.x, ObjectRgb->relativePosition.y, ObjectRgb->relativePosition.z);
        // Object position from model matrix of object + the relative position of the collider
        glm::vec3 objectPosition = glm::vec3(object->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1)) + relative_glm;
        // vector from portal center to object center
        glm::vec4 portalToObj(objectPosition - portalPosition, 0);
        float dot = glm::dot(portalToObj, portalNormal);
        return dot;
    }
    bool Portal::addToPassing(r3d::Collider* objectCollider, const std::string& objectName) {
        // if object is already in passedObjects then no need to add it again
        if(passedObjects.find(objectName) != passedObjects.end()) return false;
        // if its in markedForRemoval then it has already been teleported
        // and will be removed next call to update()
        if(markedForRemoval.find(objectName) != markedForRemoval.end()) return false;
        // put pointer as a std::shared_ptr
        // shared_ptr: is used to make sure that the pointer is not deleted 
        // when another pointer is still pointing to it to not delete a collider that
        // is pointed to by the physicsL library by mistake
        passedObjects[objectName] = std::make_shared<r3d::Collider*>(objectCollider);
        // Add object to failSafeTeleportLocation
        calculateFailSafeLocation(objectName);
        return true;
    }

    void Portal::update() {
        if(!surface || !destination || !surfaceCollider) return;
        // if no passedObjects and no markedForRemoval
        // then make sure that the surface collider is not a trigger
        // as there is no need for it to be a trigger (handles thread safety)
        if(passedObjects.empty() && markedForRemoval.empty()) {
            surfaceCollider->setIsTrigger(false);
            return;
        }
        // clears markedForRemoval objects
        for (auto& objectName : markedForRemoval) {
            r3d::Collider *coll = dynamic_cast<r3d::Collider *>(*passedObjects[objectName].get());
            if(coll->getIsTrigger())coll->setIsTrigger(false);
            passedObjects.erase(objectName);
            failSafeTeleportLocation.erase(objectName);
        }
        markedForRemoval.clear();
        // loop over passedObjects and send them to passObject
        // to handles teleportation if needed
        for(auto& [objectName, collider] : passedObjects) {
            passObject(*(collider.get()), objectName);
        }
    }

    bool Portal::passObject(r3d::Collider* objectCollider, const std::string& objectName) {
        // if no destination then do nothing
        if(!destination) return false;
        // Check if object is to be disabled or surface
        // for cases when portal is on ground/ceiling
        if(togObj) {
            // if on ground/ceiling then disable object collider
            objectCollider->setIsTrigger(true);
        }
        else {
            // if on wall then disable surface collider
            surfaceCollider->setIsTrigger(true);
        }
        float dot = hasPassed(objectName);
        if (dot < 0.0f && dot > -2.0f) {
            // if object has passed then teleport it
            teleportObject(objectName);
            // revert back collision correctly
            if(togObj) {
                objectCollider->setIsTrigger(false);
            }
            else {
                surfaceCollider->setIsTrigger(false);
            }
            // add object to be removed
            assertRemoval(objectName);
            return true;
        } else if (dot > 2.0f || dot < -2.0f) {
            // object too far away from portal
            if(togObj) {
                objectCollider->setIsTrigger(false);
            }
            else {
                surfaceCollider->setIsTrigger(false);
            }
            // add object to be removed
            assertRemoval(objectName);
        }
        return false;
    }

    void Portal::teleportObject(const std::string& ObjectName) {
        // if no destination then do nothing
        if(destination == nullptr) return;
        // get object and check if it is valid
        Entity* object = getWorld()->getEntityByName(ObjectName);
        if(object == surface) return;
        if(object == nullptr) return;
        objectRgb = object->getComponent<RigidBodyComponent>();
        if(objectRgb == nullptr) return;
        if(objectRgb->getBody()->getType() == r3d::BodyType::STATIC) return;

        // Call teleportation functions
        // on position, orientation, and velocity
        r3d::Vector3 newPosition = teleportedPosition(ObjectName, object->localTransform.getPosition());
        r3d::Quaternion newRotation = teleportedRotation(object->localTransform.getRotation(), ObjectName);
        r3d::Vector3 newVelocity = teleportedVelocity(objectRgb->getBody()->getLinearVelocity());
        
        // Set new position, orientation, and velocity
        object->localTransform.setTransform(r3d::Transform(newPosition, newRotation));
        // for rigid body need to add relative position of rigid body
        objectRgb->getBody()->setTransform(r3d::Transform(newPosition + objectRgb->relativePosition, newRotation));
        // set velocity after moving object (handles physics library cringe)
        objectRgb->getBody()->setLinearVelocity(newVelocity);
    }

    r3d::Vector3 Portal::teleportedVelocity(const r3d::Vector3& objectVelocity) const {
        glm::vec4 objectVelocity_glm(objectVelocity.x, objectVelocity.y, objectVelocity.z, 0);
        // velocity is a vector thus we only need to apply the rotation to it
        glm::vec4 relativeVel = invPortalRot * objectVelocity_glm;
        relativeVel = halfTurn * relativeVel;
        glm::vec4 temp = destination->portalRot * relativeVel;
        return r3d::Vector3(temp.x, temp.y, temp.z);
    }

    r3d::Quaternion Portal::teleportedRotation(const r3d::Quaternion& objectRotation, const std::string& objectName) const {
        glm::fquat objectRotation_glm(objectRotation.w, objectRotation.x, objectRotation.y, objectRotation.z);
        // apply rotation to quaternion of object
        glm::fquat relativeRot = invPortalRot * objectRotation_glm;
        relativeRot = halfTurn * relativeRot;
        glm::fquat newObjectRotation_glm = destination->portalRot * relativeRot;
        // make sure it's normalized
        newObjectRotation_glm = glm::normalize(newObjectRotation_glm);
        // handles the case to make the player camera up is always (0, 1, 0)
        // so player doesn't get disorented when 
        // entering through a portal on ground/ceiling 
        // and exiting through a portal on a wall
        if (objectName == "Player" && ((std::abs(portalNormal.y) > 0.7f) ^ (std::abs(destination->portalNormal.y) > 0.7f))) {
            // If objectName is "Player" and one of the portals is on ground/ceiling and other is not (XOR)
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
            // Move player to new axis and force up direction
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
        // Normalize quaternion and convert to reactphysics3d quaternion
        newObjectRotation_glm = glm::normalize(newObjectRotation_glm);
        return r3d::Quaternion(newObjectRotation_glm.x, newObjectRotation_glm.y, newObjectRotation_glm.z, newObjectRotation_glm.w);
    }

    r3d::Vector3 Portal::teleportedPosition(const std::string& ObjectName, const r3d::Vector3& objectPosition) const {
        glm::vec4 objectPosition_glm;
        if (failSafeTeleportLocation.count(ObjectName) && shouldUseFailSafeLocation(objectPosition)) {
            // if failSafeTeleportLocation exists and we should use it
            // then use it
            objectPosition_glm = failSafeTeleportLocation.at(ObjectName);
            objectPosition_glm.w = 1;
        } else {
            // else use object position
            objectPosition_glm = glm::vec4(objectPosition.x, objectPosition.y, objectPosition.z, 1);
        }
        // For a position we need to apply the model matrices of each of the portals
        glm::vec4 relativePos = invLocalToWorld * objectPosition_glm;
        relativePos = halfTurn * relativePos;
        glm::vec4 temp = destination->localToWorld * relativePos;
        // if any of them is on ground/ceiling then we need to move the object
        // in direction of destination normal to avoid clipping through ground/ceiling
        if(std::abs(portalNormal.y) > 0.7f || std::abs(destination->portalNormal.y) > 0.7f) {
            // get bounds of object
            r3d::Vector3 min, max;
            objectRgb->getCollider()->getCollisionShape()->getLocalBounds(min, max);
            // get height
            float height = max.y - min.y;
            // move in the direction of normal with magnitude depending
            // on object collider height
            temp += destination->portalNormal * height;
        } else {
            // if both not on ground/ceiling then move in direction of normal
            // by 0.5f to avoid re-teleportation
            temp += destination->portalNormal * 0.5f;
        }
        return r3d::Vector3(temp.x, temp.y, temp.z);
    }

    void Portal::calculateCachedValues() {
        // Gets called when destination is set
        // Computes constant values that would be same for every teleportation
        const r3d::Quaternion& rot = localTransform.getRotation();
        portalRot = glm::fquat(rot.w, rot.x, rot.y, rot.z);
        invPortalRot = glm::inverse(portalRot);
        localToWorld = getLocalToWorldMatrix();
        invLocalToWorld = glm::inverse(localToWorld);
        portalNormal = glm::vec4(portalRot * glm::vec3(0, 0, 1), 0);
        portalPosition = glm::vec3(localToWorld * glm::vec4(0, 0, 0, 1));
        togObj = std::abs(portalNormal.y) > 0.7f;
    }
    void Portal::getSurface() {
        // RayCast behind the portal to get the surface
        // the portal is currently on (if any)
        std::string surfaceName = "";
        RayCastGetSurface rayCastHandler(surfaceName);
        // Ray Cast from center of portal to behind it (in direction of -ve normal)
        glm::vec4 start(localToWorld * glm::vec4(0, 0, 0, 1));
        // Note: portalNormal is front facing
        glm::vec4 end = start - portalNormal;
        r3d::Ray ray(r3d::Vector3(start.x, start.y, start.z), r3d::Vector3(end.x, end.y, end.z));
        // Cast ray with rayCastHandler
        getWorld()->getPhysicsWorld()->raycast(ray, &rayCastHandler);
        // if surfaceName got modified then we found a surface
        if(surfaceName != "") {
            // Set the surface with entity with name surfaceName
            Entity* surface = getWorld()->getEntityByName(surfaceName);
            if(surface != nullptr) {
                setSurface(surface);
            }
        }
    }
    void Portal::assertRemoval(const std::string &objectName) {
        // Function for avoiding issues with multi-threading
        // makes sure that collision won't be ignored
        // if multiple threads accessed addToPassing at the same time
        if(passedObjects.count(objectName)) {
            markedForRemoval.insert(objectName);
        }
    }
}