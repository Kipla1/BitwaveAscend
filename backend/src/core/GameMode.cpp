#include "core/GameMode.h"

namespace bitwave::core {

shared::GameState GameMode::snapshot() const {
    const auto p=player_.position(), v=player_.velocity();
    shared::GameState out;
    out.playerX=p.x; out.playerY=p.y;
    out.velocityX=v.x; out.velocityY=v.y;
    out.health=player_.health(); out.score=player_.score();
    out.gameOver=gameOver_; out.playerSkinId=player_.skinId();
    return out;
}

void GameMode::setDirection(shared::PlayerAction action, bool pressed) {
    switch (action) {
        case shared::PlayerAction::MoveLeft: left_=pressed; break;
        case shared::PlayerAction::MoveRight: right_=pressed; break;
        case shared::PlayerAction::MoveForward: forward_=pressed; break;
        case shared::PlayerAction::MoveBackward: backward_=pressed; break;
        default: break;
    }
}

float GameMode::playerSpeedMultiplier() const {
    if (difficulty_==shared::Difficulty::Easy) return 1.15f;
    if (difficulty_==shared::Difficulty::Hard) return .9f;
    return 1.0f;
}

float GameMode::enemySpeedMultiplier() const {
    if (difficulty_==shared::Difficulty::Easy) return .75f;
    if (difficulty_==shared::Difficulty::Hard) return 1.3f;
    return 1.0f;
}

float GameMode::obstacleSpeedMultiplier() const {
    if (difficulty_==shared::Difficulty::Easy) return .8f;
    if (difficulty_==shared::Difficulty::Hard) return 1.25f;
    return 1.0f;
}

} // namespace bitwave::core
