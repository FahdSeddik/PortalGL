#pragma once
#include "entity.hpp"
namespace r3d = reactphysics3d;
namespace portal {
    class r3d::Collider;
    class Door : public Entity {
        // To store current status of the door
        bool isOpened = false;
        // Indicates whether setting up animation callbacks is done or not
        bool setupDone = false;
        // Store direct access to door collider
        r3d::Collider *collider = nullptr;
        // To be called once in case of animation is called
        void setup();
        // Make sure EntityFactory can access Door constructor
        friend class EntityFactory;
        Door() : Entity() { canHoldPortal = false;}

    public:
        // Open the door if it is closed and calls appropriate animations
        void open();
        // Close the door if it is opened and calls appropriate animations
        void close();
        
        bool getIsOpened() const { return isOpened; }
        virtual EntityFactory::EntityType getType() const override { return EntityFactory::EntityType::Door; }
    };
}