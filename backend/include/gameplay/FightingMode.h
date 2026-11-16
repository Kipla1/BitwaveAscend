#pragma once

#include "core/GameMode.h"

namespace bitwave::gameplay {

class FightingMode final : public core::GameMode {
public:
    shared::GameModeType type() const override { return shared::GameModeType::Fighting; }
    void start() override;
    void handleAction(shared::PlayerAction action, bool pressed) override;
    void update(float deltaTime) override;
    bool finished() const override { return gameOver_; }
private:
    float attackCooldown_{0};
};

} // namespace bitwave::gameplay
