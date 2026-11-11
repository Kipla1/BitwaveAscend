#include "entities/Player.h"

namespace bitwave::entities {

Player::Player() : Character("Player", "skin.default", {80, 280}, 100, 220) {}

void Player::jump() {
    auto v = velocity();
    if (position().y >= 568) { v.y = -480; setVelocity(v); }
}

void Player::flap() {
    auto v = velocity();
    v.y = -330;
    setVelocity(v);
}

} // namespace bitwave::entities
