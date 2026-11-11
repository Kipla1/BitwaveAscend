#include "entities/Character.h"

#include <algorithm>
#include <utility>

namespace bitwave::entities {

Character::Character(std::string name, std::string resourceKey, Vec2 position,
                     int maxHealth, float speed)
    : Entity(position), name_(std::move(name)), resourceKey_(std::move(resourceKey)),
      health_(maxHealth), maxHealth_(maxHealth), speed_(speed) {}

void Character::takeDamage(int amount) { health_ = std::max(0, health_ - std::max(0, amount)); }
void Character::heal(int amount) { health_ = std::min(maxHealth_, health_ + std::max(0, amount)); }

} // namespace bitwave::entities
