#pragma once

#include "entities/Character.h"

namespace bitwave::entities {

class Enemy final : public Character {
public:
    Enemy(std::string name = "Training Bot",
          std::string resourceKey = "enemy.training_bot",
          Vec2 position = {360, 280});
    void chase(const Entity& target, float deltaTime, float speedMultiplier = 1.0f);
};

} // namespace bitwave::entities
