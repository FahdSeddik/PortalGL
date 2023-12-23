#pragma once
#include "entity.hpp"

namespace portal {
    class r3d::RigidBody;
    class Elevator : public Entity {
        friend class EntityFactory;
        // TODO: make elevator call new level

        void setup();
        // RigidBody of door
        r3d::RigidBody* rgb = nullptr;
        // Indicates whether setting up animation callbacks is done or not
        bool setupDone = false;
        bool isOpened = true;
        protected:
        Elevator() : Entity() { canHoldPortal = false; }
        public:
        // To open the elevator
        void open();
        // To close the elevator
        void close();


        virtual EntityFactory::EntityType getType() const override { return EntityFactory::EntityType::Elevator; }
    };
}