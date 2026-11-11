#include "gameplay/FightingMode.h"

#include <algorithm>
#include <stdexcept>
#include "gameplay/Combat.h"
#include "gameplay/Collision.h"
#include "gameplay/Physics.h"

namespace bitwave::gameplay {

void FightingMode::start() {
    player_=entities::Player{};
    player_.setPosition({80,568});
    enemies_={entities::Enemy("Training Bot", "enemy.training_bot", {360,280})};
    obstacles_.clear(); gameOver_=false; damageCooldown_=0; attackCooldown_=0;
}

void FightingMode::handleAction(shared::PlayerAction action, bool pressed) {
    setDirection(action, pressed);
    if (pressed && action==shared::PlayerAction::Attack && attackCooldown_==0) {
        Combat::attack(player_, enemies_);
        attackCooldown_=.25f;
    }
    if (pressed && action==shared::PlayerAction::Jump) player_.jump();
}

void FightingMode::update(float dt) {
    if (dt<0) throw std::invalid_argument("negative delta time");
    if (gameOver_) return;
    damageCooldown_=std::max(0.0f, damageCooldown_-dt);
    attackCooldown_=std::max(0.0f, attackCooldown_-dt);
    const float x=float(right_)+float(forward_)-float(left_)-float(backward_);
    auto v=player_.velocity();
    v.x=x*player_.speed()*playerSpeedMultiplier();
    player_.setVelocity(v);
    Physics::step(player_,dt,1200);
    for (auto& enemy : enemies_) if (enemy.alive()) {
        enemy.chase(player_, dt, enemySpeedMultiplier());
        if (damageCooldown_==0 && Collision::overlaps(player_, enemy)) {
            player_.takeDamage(10); damageCooldown_=.5f;
        }
    }
    const bool defeated=std::all_of(enemies_.begin(), enemies_.end(),
                                    [](const auto& enemy){ return !enemy.alive(); });
    gameOver_=!player_.alive() || defeated;
}

} // namespace bitwave::gameplay
