#include "core/Game.h"

#include <stdexcept>
#include "gameplay/FightingMode.h"
#include "gameplay/FlappyMode.h"
#include "gameplay/RunningMode.h"

namespace bitwave::core {

Game::Game(shared::GameModeType type, shared::Difficulty difficulty)
    : difficulty_(difficulty) {
    switch (type) {
        case shared::GameModeType::Fighting: mode_=std::make_unique<gameplay::FightingMode>(); break;
        case shared::GameModeType::Running: mode_=std::make_unique<gameplay::RunningMode>(); break;
        case shared::GameModeType::Flappy: mode_=std::make_unique<gameplay::FlappyMode>(); break;
        default: throw std::invalid_argument("unknown game mode");
    }
}

void Game::start() {
    paused_=false;
    mode_->setDifficulty(difficulty_);
    mode_->start();
    mode_->setSkinId(skinId_);
}

void Game::handleAction(shared::PlayerAction action, bool pressed) {
    if (action==shared::PlayerAction::Pause && pressed) { paused_=!paused_; return; }
    if (!paused_ || !pressed) mode_->handleAction(action, pressed);
}

void Game::update(float dt) { if (!paused_) mode_->update(dt); }
bool Game::finished() const { return mode_->finished(); }
shared::GameState Game::state() const { auto out=mode_->snapshot(); out.paused=paused_; return out; }
void Game::setSkinId(std::int64_t id) { skinId_=id; mode_->setSkinId(id); }

} // namespace bitwave::core
