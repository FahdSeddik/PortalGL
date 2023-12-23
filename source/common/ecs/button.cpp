#include "button.hpp"
#include "door.hpp"
#include "world.hpp"
#include <GLFW/glfw3.h>
namespace portal {
    
    void Button::press() {
        if(!isPressed) {
            // Start animation and call onPress
            isPressed = true;
            getWorld()->startAnimation(name + "_" + "press");
            if(onPress) onPress();
        }
        // Increase count of entities on button
        countOnButton++;
    }

    void Button::release() {
        if(isPressed && countOnButton == 1) {
            // If only one entity remain and release is called then release button
            // Start animation and call onRelease
            isPressed = false;
            getWorld()->startAnimation(name + "_" + "press", true);
            if(onRelease) onRelease();
        }
        countOnButton--;
    }

    void Button::deserializeAction(const nlohmann::json &data) {
        if(!data.is_object()) return;
        // Set onPress and onRelease
        
        // If action has "Door" then button will open and close a door
        if(data.contains("Door")) {
            std::string doorName = data["Door"];
            // Call open and close in onPress and onRelease
            onPress = [doorName, this]() {
                Door *door = dynamic_cast<Door*>(getWorld()->getEntityByName(doorName));
                door->open();
            };
            onRelease = [doorName, this]() {
                Door *door = dynamic_cast<Door*>(getWorld()->getEntityByName(doorName));
                door->close();
            };
        }
    }

    void Button::deserialize(const nlohmann::json &data) {
        // Call parent deserialize
        Entity::deserialize(data);
        // Deserialize action
        if(data.contains("action")){
            deserializeAction(data["action"]);
        }
    }
}