#pragma once
#include "entity.hpp"
namespace portal {
    class Button : public Entity {
        // Whether the button is pressed or not
        bool isPressed = false;
        // Count of entities currently colliding with the button
        int countOnButton = 0;
        // Callback functions for when the button is pressed or released
        std::function<void()> onPress = nullptr;
        std::function<void()> onRelease = nullptr;
        // Deserialize action
        void deserializeAction(const nlohmann::json &data);
        // Make sure EntityFactory can access Button constructor
        friend class EntityFactory;
        Button() : Entity() {canHoldPortal = false;}
        public:
        // onPress and onRelease button
        void press();
        void release();
        
        bool getIsPressed() const { return isPressed; }
        virtual EntityFactory::EntityType getType() const override { return EntityFactory::EntityType::Button; }
        virtual void deserialize(const nlohmann::json &data) override;
    };
}