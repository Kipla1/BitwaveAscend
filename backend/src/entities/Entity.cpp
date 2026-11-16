#include "entities/Entity.h"

namespace bitwave::entities {

Entity::Entity(Vec2 position, Vec2 size) : position_(position), size_(size) {}

bool Entity::overlaps(const Entity& other) const {
    const auto a = position_, b = other.position_;
    return a.x < b.x + other.size_.x && a.x + size_.x > b.x &&
           a.y < b.y + other.size_.y && a.y + size_.y > b.y;
}

} // namespace bitwave::entities
