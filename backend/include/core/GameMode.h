#pragma once

#include <vector>
#include "entities/Enemy.h"
#include "entities/Obstacle.h"
#include "entities/Player.h"
#include "shared/Contracts.h"

namespace bitwave::core {

class GameMode {
public:
    virtual ~GameMode() = default;
    virtual shared::GameModeType type() const = 0;
    virtual void start() = 0;
    virtual void handleAction(shared::PlayerAction action, bool pressed) = 0;
    virtual void update(float deltaTime) = 0;
    virtual bool finished() const = 0;
    shared::GameState snapshot() const;
    void setSkinId(std::int64_t id) { player_.setSkinId(id); }
    void setDifficulty(shared::Difficulty value) { difficulty_=value; }

protected:
    entities::Player player_;
    std::vector<entities::Enemy> enemies_;
    std::vector<entities::Obstacle> obstacles_;
    bool left_{false}, right_{false}, forward_{false}, backward_{false};
    bool gameOver_{false};
    shared::Difficulty difficulty_{shared::Difficulty::Medium};
    float damageCooldown_{0};
    void setDirection(shared::PlayerAction action, bool pressed);
    float playerSpeedMultiplier() const;
    float enemySpeedMultiplier() const;
    float obstacleSpeedMultiplier() const;
};

} // namespace bitwave::core
