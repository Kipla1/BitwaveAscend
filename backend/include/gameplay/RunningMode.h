#pragma once

#include "core/GameMode.h"

namespace bitwave::gameplay {

class RunningMode final : public core::GameMode {
public:
    shared::GameModeType type() const override { return shared::GameModeType::Running; }
    void start() override;
    void handleAction(shared::PlayerAction action, bool pressed) override;
    void update(float deltaTime) override;
    bool finished() const override { return gameOver_; }
};

} // namespace bitwave::gameplay
