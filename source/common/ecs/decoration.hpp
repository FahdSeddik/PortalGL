#pragma once
#include "entity.hpp"

namespace portal {
    // Class decoration is a subclass of attachable
    // so entities such as "Radio", "Papers", etc. are decorations
    // NOTE: static decorations should be treated as regular entitiess
    class Decoration : public Attachable {
        friend class EntityFactory;
        protected:
        Decoration() : Attachable() {}
        public:
        virtual EntityFactory::EntityType getType() const override { return EntityFactory::EntityType::Decoration; }
    };
}