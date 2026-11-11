#pragma once

#include <string>
#include "entities/Entity.h"

namespace bitwave::entities {

// Shared stats and artwork key for playable and enemy characters.
class Character : public Entity {
public:
    Character(std::string name, std::string resourceKey, Vec2 position,
              int maxHealth, float speed);
    virtual ~Character() = default;

    const std::string& name() const { return name_; }
    const std::string& resourceKey() const { return resourceKey_; }
    int health() const { return health_; }
    int maxHealth() const { return maxHealth_; }
    float speed() const { return speed_; }
    bool alive() const { return health_ > 0; }
    void takeDamage(int amount);
    void heal(int amount);

private:
    std::string name_, resourceKey_;
    int health_, maxHealth_;
    float speed_;
};

} // namespace bitwave::entities
