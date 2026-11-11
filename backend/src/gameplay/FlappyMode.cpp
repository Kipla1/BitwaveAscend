#include "gameplay/FlappyMode.h"

#include <stdexcept>
#include "gameplay/Collision.h"
#include "gameplay/Physics.h"

namespace bitwave::gameplay {

void FlappyMode::start() {
    player_=entities::Player{};
    player_.setPosition({100,280});
    player_.setVelocity({150,0});
    enemies_.clear();
    obstacles_={entities::Obstacle({360,0},{42,250}),
                entities::Obstacle({620,330},{42,270})};
    gameOver_=false;
}

void FlappyMode::handleAction(shared::PlayerAction action, bool pressed) {
    setDirection(action, pressed);
    if (pressed && action==shared::PlayerAction::Flap) player_.flap();
}

void FlappyMode::update(float dt) {
    if (dt<0) throw std::invalid_argument("negative delta time");
    if (gameOver_) return;
    const float direction=float(right_)+float(forward_)-float(left_)-float(backward_);
    const float xSpeed=(150.0f+direction*player_.speed())*playerSpeedMultiplier();
    player_.setVelocity({xSpeed,player_.velocity().y});
    if (Physics::step(player_,dt,900) || player_.position().y<=0) {
        gameOver_=true; return;
    }
    auto p=player_.position();
    if (p.x>=768) { p.x=40; player_.addScore(); player_.setPosition(p); }
    for (auto& obstacle : obstacles_) {
        auto op=obstacle.position(); op.x-=130*obstacleSpeedMultiplier()*dt;
        if (op.x < -obstacle.size().x) op.x=800;
        obstacle.setPosition(op);
        if (Collision::overlaps(player_,obstacle)) { gameOver_=true; break; }
    }
}

} // namespace bitwave::gameplay
