#pragma once

#include <memory>
#include "core/GameMode.h"

namespace bitwave::core {

// One selected mode and its current run.
class Game {
public:
    Game(shared::GameModeType type, shared::Difficulty difficulty);
    void start();
    void handleAction(shared::PlayerAction action, bool pressed);
    void update(float deltaTime);
    bool finished() const;
    shared::GameState state() const;
    void setSkinId(std::int64_t id);

private:
    std::unique_ptr<GameMode> mode_;
    shared::Difficulty difficulty_;
    std::int64_t skinId_{0};
    bool paused_{false};
};

} // namespace bitwave::core
