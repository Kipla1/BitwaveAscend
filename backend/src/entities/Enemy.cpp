#include "entities/Enemy.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace bitwave::entities {

Enemy::Enemy(std::string name, std::string resourceKey, Vec2 position)
    : Character(std::move(name), std::move(resourceKey), position, 40, 100) {}

void Enemy::chase(const Entity& target, float dt, float multiplier) {
    if (dt <= 0) return;
    auto p = position();
    const auto t = target.position();
    const float dx = t.x - p.x, dy = t.y - p.y;
    const float length = std::sqrt(dx*dx + dy*dy);
    if (length > 0) {
        const float step = std::min(length, speed()*multiplier*dt);
        p.x += dx/length*step;
        p.y += dy/length*step;
        setPosition(p);
    }
}

} // namespace bitwave::entities
