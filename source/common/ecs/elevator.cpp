#include "elevator.hpp"
#include "world.hpp"
#include "../components/RigidBody.hpp"
namespace portal {
    void Elevator::setup() {
        // Retrieve rigid body 
        // The elevator has several rigidbody components but the first one will return (the correct one)
        rgb = getComponent<RigidBodyComponent>()->getBody();
    }

    void Elevator::open() {
        if(!isOpened) {
            if(!setupDone) {
                // make sure rigid body is setup
                setup();
                setupDone = true;
            }
            isOpened = true;
            // Same as close()
            r3d::Transform door = rgb->getTransform();
            door.setPosition(localTransform.getPosition() + localTransform.getRotation() * r3d::Vector3(0, 5, 3.5));
            rgb->setTransform(door);
            // Call same animation but reversed
            getWorld()->startAnimation(name + "_" + "left_close", true);
            getWorld()->startAnimation(name + "_" + "right_close", true);
        }
    }


    void Elevator::close() {
        if(isOpened) {
            if(!setupDone) {
                // make sure rigid body is setup
                setup();
                setupDone = true;
            }
            isOpened = false;
            // Get the elevator door transform
            r3d::Transform door = rgb->getTransform();
            // Set the position for rigid body to be in front
            // of the elevator to prevent player from phazing through the door
            // NOTE: this couldn't be set as trigger and set back as not trigger due to us using triggers to close the elevator
            door.setPosition(localTransform.getPosition() + localTransform.getRotation() * r3d::Vector3(0, 5, -3.5));
            rgb->setTransform(door);
            // Call appropriate animations
            getWorld()->startAnimation(name + "_" + "left_close", false);
            getWorld()->startAnimation(name + "_" + "right_close", false);
        }
    }


}

