#include "core/GameManager.h"

namespace bitwave::core {

void GameManager::startMode(shared::GameModeType type, shared::Difficulty difficulty) {
    game_=std::make_unique<Game>(type,difficulty);
    game_->start();
    game_->setSkinId(skinId_);
}

void GameManager::handleAction(shared::PlayerAction action, bool pressed) {
    if (game_) game_->handleAction(action, pressed);
}

void GameManager::update(float dt) {
    if (game_) game_->update(dt);
}

shared::GameState GameManager::state() const {
    return game_ ? game_->state() : shared::GameState{};
}

void GameManager::setSkinId(std::int64_t itemId) {
    skinId_=itemId;
    if (game_) game_->setSkinId(itemId);
}

} // namespace bitwave::core
