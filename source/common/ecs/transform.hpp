#pragma once

#include <glm/glm.hpp>
#include <json/json.hpp>
#include <glm/gtc/quaternion.hpp>
#include <reactphysics3d/reactphysics3d.h>
namespace r3d = reactphysics3d;

namespace portal {

    // A transform defines the translation, rotation & scale of an object relative to its parent
    struct Transform {
        r3d::Transform transform;
    public:
        glm::vec3 scale = glm::vec3(1, 1, 1); // The scale is defined as a vec3. (1,1,1) means no scaling.

        // This function computes and returns a matrix that represents this transform
        glm::mat4 toMat4() const;
         // Deserializes the entity data and components from a json object
        void deserialize(const nlohmann::json&);

        const r3d::Transform& getTransform() const {
            return transform;
        }

        const r3d::Vector3& getPosition() const {
            return transform.getPosition();
        }

        const r3d::Quaternion& getRotation() const {
            return transform.getOrientation();
        }

        void setPosition(const glm::vec3 position) {
            transform.setPosition(r3d::Vector3(position.x, position.y, position.z));
        }

        void setPosition(const r3d::Vector3 position) {
            transform.setPosition(position);
        }

        void setRotation(const glm::quat rotation) {
            transform.setOrientation(r3d::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w));
        }

        void setRotation(const r3d::Quaternion rotation) {
            transform.setOrientation(rotation);
        }

        void setTransform(const r3d::Transform transform) {
            this->transform = transform;
        }

        static r3d::Transform interpolate(const Transform& a, const Transform& b, float t) {
            r3d::Transform transform;
            transform.setPosition(r3d::Vector3(glm::mix(a.getPosition().x, b.getPosition().x, t), glm::mix(a.getPosition().y, b.getPosition().y, t), glm::mix(a.getPosition().z, b.getPosition().z, t)));
            transform.setOrientation(r3d::Quaternion(glm::mix(a.getRotation().x, b.getRotation().x, t), glm::mix(a.getRotation().y, b.getRotation().y, t), glm::mix(a.getRotation().z, b.getRotation().z, t), glm::mix(a.getRotation().w, b.getRotation().w, t)));
            return transform;
        }
    };

}