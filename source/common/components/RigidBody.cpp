#include "../deserialize-utils.hpp"
#include <reactphysics3d/reactphysics3d.h>
#include "RigidBody.hpp"
#include "../ecs/world.hpp"

namespace portal {
    void RigidBodyComponent::deserialize(const nlohmann::json& data) {
        if(!data.is_object()) return;
        r3d::PhysicsWorld *pWorld = this->getOwner()->getWorld()->getPhysicsWorld();
        relativePosition = r3d::Vector3(0,0,0);
        glm::vec3 rel = data.value("relativePosition", glm::vec3(0.0f));
        relativePosition = r3d::Vector3(rel.x, rel.y, rel.z);
        r3d::Transform transform = this->getOwner()->localTransform.getTransform();
        transform.setPosition(transform.getPosition() + relativePosition);
        // Create a rigid body in the physics world
        r3d::RigidBody *body = pWorld->createRigidBody(transform);
        // Set the rigid body to the component
        this->body = body;
        this->body->setUserData(new std::string((this->getOwner()->name.empty()) ? std::to_string(pWorld->getNbRigidBodies()):this->getOwner()->name));
        // get r3dType
        const std::string r3dType = data.value("r3dType", "Static");
        if(r3dType == "Kinemtatic") {
            this->body->setType(r3d::BodyType::KINEMATIC);
        } else if (r3dType == "Dynamic") {
            this->body->setType(r3d::BodyType::DYNAMIC);
        } else {
            this->body->setType(r3d::BodyType::STATIC);
        }
        // get enableGravity
        const bool enableGravity = data.value("enableGravity", false);
        this->body->enableGravity(enableGravity);

        // get allowedToSleep
        const bool allowedToSleep = data.value("allowedToSleep", false);
        this->body->setIsAllowedToSleep(allowedToSleep);

        // motion axis
        const glm::vec3 motionAxis = data.value("motionAxis", glm::vec3(1.0f));
        this->body->setLinearLockAxisFactor(r3d::Vector3(motionAxis.x, motionAxis.y, motionAxis.z));
        //rotation axis
        const glm::vec3 rotationAxis = data.value("rotationAxis", glm::vec3(1.0f));
        this->body->setAngularLockAxisFactor(r3d::Vector3(rotationAxis.x, rotationAxis.y, rotationAxis.z));
        
        // parse collider
        if(data.contains("collider")) {
            deserialize_collider(data["collider"]);
        }
    }

    void RigidBodyComponent::deserialize_collider(const nlohmann::json& data) {
        if(!data.is_object()) return;
        r3d::PhysicsCommon& pCommon = this->getOwner()->getWorld()->getPhysicsCommon();
        std::string colliderType = data.value("type", "");
        if(colliderType == "Box Collider") {
            // halfExtents
            const glm::vec3 hE = data.value("halfExtents", glm::vec3(1.0f));
            const r3d::Vector3 halfExtents(hE.x, hE.y, hE.z);
            // Create a box shape and add it to the body
            r3d::BoxShape* boxShape = pCommon.createBoxShape(halfExtents);
            collider = body->addCollider(boxShape, r3d::Transform::identity());
        }else if(colliderType == "Sphere Collider") {
            // radius
            const r3d::decimal radius = data.value("radius", 1.0f);
            // Create a sphere shape and add it to the body
            r3d::SphereShape* sphereShape = pCommon.createSphereShape(radius);
            collider = body->addCollider(sphereShape, r3d::Transform::identity());
        } else if(colliderType == "Capsule Collider") {
            // radius
            const r3d::decimal radius = data.value("radius", 1.0f);
            // height
            const r3d::decimal height = data.value("height", 2.0f);
            // Create a capsule shape and add it to the body
            r3d::CapsuleShape* capsuleShape = pCommon.createCapsuleShape(radius, height);
            collider = body->addCollider(capsuleShape, r3d::Transform::identity());
        }
        // material properties
        r3d::Material &material = collider->getMaterial();
        material.setBounciness(data.value("bounciness", material.getBounciness()));
        material.setFrictionCoefficient(data.value("friction", material.getFrictionCoefficient()));
        material.setMassDensity(data.value("massDensity", material.getMassDensity()));
        // trigger
        collider->setIsTrigger(data.value("isTrigger", false));
    }

    RigidBodyComponent::~RigidBodyComponent() {
        if (this->body) {
            this->getOwner()->getWorld()->getPhysicsWorld()->destroyRigidBody(body);
        }
    }
}