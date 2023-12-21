#pragma once
#include <string>
#include <unordered_map>
namespace portal {
    class Entity;
    // Factory pattern to create entities
    class EntityFactory {
        public:
            enum class EntityType
            {
                Regular,
                Portal,
                Door,
                Button,
                Cube,
                Player,
                Decoration,
                // Add other types as needed

                // Switch,
            };
            // Create an entity of the given type
            static Entity *createEntity(EntityType type);

            // Convert an entity type to a string
            static std::string entityTypeToString(EntityType type) {
                static std::unordered_map<EntityType, std::string> entityTypeMap = {
                    {EntityType::Regular, "Regular"},
                    {EntityType::Portal, "Portal"},
                    {EntityType::Door, "Door"},
                    {EntityType::Button, "Button"},
                    {EntityType::Cube, "Cube"},
                    {EntityType::Player, "Player"},
                    {EntityType::Decoration, "Decoration"}
                    // Add other types as needed

                    // {EntityType::Switch, "Switch"},
                };
                // Default to regular entity type
                return entityTypeMap.count(type) ? entityTypeMap[type] : entityTypeMap[EntityType::Regular];
            }

            // Convert a string to an entity type
            static EntityType stringToEntityType(const std::string &type) {
                static std::unordered_map<std::string, EntityType> entityTypeMap = {
                    {"Regular", EntityType::Regular},
                    {"Portal", EntityType::Portal},
                    {"Door", EntityType::Door},
                    {"Button", EntityType::Button},
                    {"Cube", EntityType::Cube},
                    {"Player", EntityType::Player},
                    {"Decoration", EntityType::Decoration}
                    // Add other types as needed

                    // {"Switch", EntityType::Switch},
                };
                // Default to regular entity type
                return entityTypeMap.count(type) ? entityTypeMap[type] : EntityType::Regular;
            }
    };
}