#pragma once

#include <memory>
#include "core/Game.h"

namespace bitwave::core {

class GameManager {
public:
    void startMode(shared::GameModeType type,
                   shared::Difficulty difficulty = shared::Difficulty::Medium);
    void handleAction(shared::PlayerAction action, bool pressed = true);
    void update(float deltaTime);
    bool hasMode() const { return static_cast<bool>(game_); }
    shared::GameState state() const;
    void setSkinId(std::int64_t itemId);

private:
    std::unique_ptr<Game> game_;
    std::int64_t skinId_{0};
};

} // namespace bitwave::core
