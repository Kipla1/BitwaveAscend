#include "gameplay/RunningMode.h"

#include <algorithm>
#include <stdexcept>
#include "gameplay/Collision.h"
#include "gameplay/Physics.h"

namespace bitwave::gameplay {

void RunningMode::start() {
    player_=entities::Player{};
    player_.setPosition({80,568});
    enemies_.clear();
    obstacles_={entities::Obstacle({340,530},{36,38}),
                entities::Obstacle({620,520},{36,48})};
    gameOver_=false; damageCooldown_=0;
}

void RunningMode::handleAction(shared::PlayerAction action, bool pressed) {
    setDirection(action, pressed);
    if (pressed && action==shared::PlayerAction::Jump) player_.jump();
}

void RunningMode::update(float dt) {
    if (dt<0) throw std::invalid_argument("negative delta time");
    if (gameOver_) return;
    damageCooldown_=std::max(0.0f, damageCooldown_-dt);
    const float speed=player_.speed()*playerSpeedMultiplier();
    const float direction=float(right_)+float(forward_)-float(left_)-float(backward_);
    auto velocity=player_.velocity();
    velocity.x=std::max(-220.0f,140.0f+direction*speed);
    player_.setVelocity(velocity); // Keep the jump's vertical speed.
    Physics::step(player_,dt,1200);
    auto pp=player_.position(); pp.y=std::min(pp.y,568.0f); player_.setPosition(pp);

    for (auto& obstacle : obstacles_) {
        auto p=obstacle.position(); p.x-=180*obstacleSpeedMultiplier()*dt;
        if (p.x < -obstacle.size().x) { p.x=800; player_.addScore(); }
        obstacle.setPosition(p);
        if (damageCooldown_==0 && Collision::overlaps(player_,obstacle)) {
            player_.takeDamage(20); damageCooldown_=.6f;
        }
    }
    gameOver_=!player_.alive();
}

} // namespace bitwave::gameplay
