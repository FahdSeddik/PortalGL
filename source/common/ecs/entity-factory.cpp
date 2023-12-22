#include "entity-factory.hpp"
#include "entity.hpp"
#include "portal.hpp"
#include "door.hpp"
#include "button.hpp"
#include "cube.hpp"
#include "player.hpp"
#include "decoration.hpp"
#include "elevator.hpp"
namespace portal {
    // Create entity based on type
    Entity *EntityFactory::createEntity(EntityType type) {
        Entity *entity = nullptr;
        switch (type) {
            case EntityType::Regular:
                entity = new Entity();
                break;
            case EntityType::Portal:
                entity = new Portal();
                break;
            case EntityType::Door:
                entity = new Door();
                break;
            case EntityType::Button:
                entity = new Button();
                break;
            case EntityType::Cube:
                entity = new Cube();
                break;
            case EntityType::Player:
                entity = new Player();
                break;
            case EntityType::Decoration:
                entity = new Decoration();
                break;
            case EntityType::Elevator:
                entity = new Elevator();
                break;
            // Add other types as needed

            // case EntityType::Switch:
            //     break;
        }
        return entity;
    }
}