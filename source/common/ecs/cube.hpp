#pragma once
#include "entity.hpp"

namespace portal {

    class Cube : public Attachable {
        friend class EntityFactory;
        Cube() : Attachable() {}
        public:
        virtual EntityFactory::EntityType getType() const override { return EntityFactory::EntityType::Cube; }
    };

}