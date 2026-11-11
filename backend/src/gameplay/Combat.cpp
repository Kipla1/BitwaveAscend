#include "gameplay/Combat.h"

#include <cmath>

namespace bitwave::gameplay {

bool Combat::attack(entities::Player& player, std::vector<entities::Enemy>& enemies,
                    float range, int damage) {
    entities::Enemy* nearest = nullptr;
    float nearestDistance = range;
    for (auto& enemy : enemies) {
        if (!enemy.alive()) continue;
        const auto p = player.position(), e = enemy.position();
        const float dx = p.x-e.x, dy = p.y-e.y;
        const float distance = std::sqrt(dx*dx+dy*dy);
        if (distance <= nearestDistance) { nearest=&enemy; nearestDistance=distance; }
    }
    if (!nearest) return false;
    nearest->takeDamage(damage);
    if (!nearest->alive()) player.addScore();
    return true;
}

} // namespace bitwave::gameplay
