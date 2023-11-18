#pragma once

#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/free-camera-controller.hpp"
#include "../components/RigidBody.hpp"

#include "../application.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include<glm/common.hpp>

namespace our
{

    // The free camera controller system is responsible for moving every entity which contains a FreeCameraControllerComponent.
    // This system is added as a slightly complex example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/free-camera-controller.hpp"
    class FreeCameraControllerSystem {
        Application* app; // The application in which the state runs
        bool mouse_locked = false; // Is the mouse locked

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app){
            this->app = app;
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent 
        void update(World* world, float deltaTime) {
            // First of all, we search for an entity containing both a CameraComponent and a FreeCameraControllerComponent
            // As soon as we find one, we break
            CameraComponent* camera = nullptr;
            FreeCameraControllerComponent *controller = nullptr;
            for(auto entity : world->getEntities()){
                camera = entity->getComponent<CameraComponent>();
                controller = entity->getComponent<FreeCameraControllerComponent>();
                if(camera && controller) break;
            }
            // If there is no entity with both a CameraComponent and a FreeCameraControllerComponent, we can do nothing so we return
            if(!(camera && controller)) return;
            // Get the entity that we found via getOwner of camera (we could use controller->getOwner())
            Entity* entity = camera->getOwner();

            // If the left mouse button is pressed, we lock and hide the mouse. This common in First Person Games.
            if(app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && !mouse_locked){
                app->getMouse().lockMouse(app->getWindow());
                mouse_locked = true;
            // If the left mouse button is released, we unlock and unhide the mouse.
            } else if(!app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && mouse_locked) {
                app->getMouse().unlockMouse(app->getWindow());
                mouse_locked = false;
            }

            // We get a reference to the entity's position and rotation
            const r3d::Vector3& pos = entity->localTransform.getPosition();
            const r3d::Quaternion& rot = entity->localTransform.getRotation();
            glm::vec3 position(pos.x, pos.y, pos.z);
            glm::quat rotation((float)rot.w, (float)rot.x, (float)rot.y, (float)rot.z);
            // If the left mouse button is pressed, we get the change in the mouse location
            // and use it to update the camera rotation
            if(app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1)){
                glm::vec2 delta = app->getMouse().getMouseDelta();
                float pitch = -delta.y * controller->rotationSensitivity; // The y-axis controls the pitch
                float yaw = -delta.x * controller->rotationSensitivity; // The x-axis controls the yaw
                glm::quat yawQuat = glm::angleAxis(yaw, glm::vec3(0,1,0));
                auto owner = camera->getOwner();
                auto M = owner->getLocalToWorldMatrix();
                glm::vec3 eye = glm::vec3(M * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                glm::vec3 center = glm::vec3(M * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f));
                glm::vec3 cameraForward = glm::normalize(center - eye);
                if(glm::dot(cameraForward, glm::vec3(0,1,0)) > 0.95f && pitch > 0) pitch = 0;
                else if (glm::dot(cameraForward, glm::vec3(0,-1,0)) > 0.95f && pitch < 0) pitch = 0;
                glm::quat pitchQuat = glm::angleAxis(pitch, glm::vec3(1,0,0));
                // Combine the pitch and yaw rotations
                rotation = yawQuat * rotation * pitchQuat;
                rotation = glm::normalize(rotation);
            }

            // We update the camera fov based on the mouse wheel scrolling amount
            float fov = camera->fovY + app->getMouse().getScrollOffset().y * controller->fovSensitivity;
            fov = glm::clamp(fov, glm::pi<float>() * 0.01f, glm::pi<float>() * 0.99f); // We keep the fov in the range 0.01*PI to 0.99*PI
            camera->fovY = fov;

            // We get the camera model matrix (relative to its parent) to compute the front, up and right directions
            glm::mat4 matrix = entity->localTransform.toMat4();

            // front: the direction the camera is looking at projected on the xz plane
            // up: global up vector (0,1,0)
            // right: the vector to the right of the camera (x-axis)
            glm::vec3 front = glm::normalize(glm::vec3(matrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
            // project on xz plane
            front.y = 0;
            front = glm::normalize(front);
            //global up
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::vec3(matrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
            // project on xz plane
            right.y = 0;
            right = glm::normalize(right);

            glm::vec3 current_sensitivity = controller->positionSensitivity;
            // If the LEFT SHIFT key is pressed, we multiply the position sensitivity by the speed up factor
            if(app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT)) current_sensitivity *= controller->speedupFactor;

            // We change the camera position based on the keys WASD/QE
            // S & W moves the player back and forth
            if(app->getKeyboard().isPressed(GLFW_KEY_W)) position += front * (deltaTime * current_sensitivity.z);
            if(app->getKeyboard().isPressed(GLFW_KEY_S)) position -= front * (deltaTime * current_sensitivity.z);
            // Q & E moves the player up and down
            if(app->getKeyboard().isPressed(GLFW_KEY_Q)) position += up * (deltaTime * current_sensitivity.y);
            if(app->getKeyboard().isPressed(GLFW_KEY_E)) position -= up * (deltaTime * current_sensitivity.y);
            // A & D moves the player left or right 
            if(app->getKeyboard().isPressed(GLFW_KEY_D)) position += right * (deltaTime * current_sensitivity.x);
            if(app->getKeyboard().isPressed(GLFW_KEY_A)) position -= right * (deltaTime * current_sensitivity.x);
            entity->localTransform.setPosition(position);
            entity->localTransform.setRotation(rotation);
            RigidBodyComponent* rgb = entity->getComponent<RigidBodyComponent>();
            if (rgb){
                // we need to get only the rotation around the y-axis
                glm::quat q = rotation;

                // Get the forward direction (z-axis) of the quaternion
                glm::vec3 forward = glm::rotate(q, glm::vec3(0.0f, 0.0f, -1.0f));

                // Remove the y-component of the forward direction
                forward.y = 0.0f;

                // Normalize the forward direction
                forward = glm::normalize(forward);

                // Compute the angle between the forward direction and the negative z-axis
                float angle = glm::acos(glm::dot(forward, glm::vec3(0.0f, 0.0f, -1.0f)));

                // Compute the cross product of the forward direction and the negative z-axis
                glm::vec3 cross = glm::cross(forward, glm::vec3(0.0f, 0.0f, -1.0f));

                // If the y-component of the cross product is negative, negate the angle
                if (cross.y < 0.0f)
                    angle = -angle;

                // Create a new quaternion representing the rotation around the y-axis
                glm::quat yRotation = glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f));
                r3d::Quaternion qt(yRotation.x, yRotation.y, yRotation.z, yRotation.w);
                r3d::Transform transform = rgb->getBody()->getTransform();
                transform.setOrientation(qt);
                transform.setPosition(entity->localTransform.getPosition());
                rgb->getBody()->setTransform(transform);
            }
        }

        // When the state exits, it should call this function to ensure the mouse is unlocked
        void exit(){
            if(mouse_locked) {
                mouse_locked = false;
                app->getMouse().unlockMouse(app->getWindow());
            }
        }

    };

}
