#pragma once

#include <vector>
#include "entities/Enemy.h"
#include "entities/Player.h"

namespace bitwave::gameplay {

class Combat {
public:
    // Hit the nearest enemy in range. Defeating one awards a point.
    static bool attack(entities::Player& player,
                       std::vector<entities::Enemy>& enemies,
                       float range = 64, int damage = 20);
};

} // namespace bitwave::gameplay
