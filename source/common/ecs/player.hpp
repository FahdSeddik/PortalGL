#pragma once
#include "entity.hpp"

namespace portal {
    class Application;
    class Player : public Entity {
        friend class EntityFactory;
        Application* app = nullptr;
        Entity* attachement = nullptr;
        std::string attachementName = "";
        // Player Vectors
        r3d::Vector3 absoluteFront;
        glm::vec3 front;
        glm::vec3 right;
        // Handles disattaching and attaching of an entity
        void checkAttachment();
        // gets the front, right, and absoluteFront of player
        void calculatePlayerVectors();
        Player() : Entity() { canHoldPortal = false, isAttachable = false; }

    public:
        void setApp(Application *app) { this->app = app; }
        // Attach an "isAttachable" entity to the player
        void attachToPlayer(Entity *entity) const;
        // To Sync player vectors & check attachment
        void update();

        const r3d::Vector3 &getAbsoluteFront() const { return absoluteFront; }
        const glm::vec3 &getFront() const { return front; }
        const glm::vec3 &getRight() const { return right; }
        Entity *getAttachement() const { return attachement; }
        virtual EntityFactory::EntityType getType() const override { return EntityFactory::EntityType::Player; }
    };

}