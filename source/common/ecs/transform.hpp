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

        const r3d::Vector3 &getPosition() const;

        const r3d::Quaternion &getRotation() const;

        void setPosition(const glm::vec3 position);

        void setPosition(const r3d::Vector3 position);

        void setRotation(const glm::quat rotation);

        void setRotation(const r3d::Quaternion rotation);

        void setTransform(const r3d::Transform transform) {
            this->transform = transform;
        }

        static r3d::Transform interpolate(const Transform &a, const Transform &b, float t);
    };

}