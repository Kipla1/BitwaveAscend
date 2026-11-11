#pragma once

#include "entities/Entity.h"

namespace bitwave::entities {

class Obstacle final : public Entity {
public:
    explicit Obstacle(Vec2 position = {}, Vec2 size = {32, 32})
        : Entity(position, size) {}
};

} // namespace bitwave::entities
