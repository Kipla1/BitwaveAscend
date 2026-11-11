#pragma once

#include <cstdint>
#include "entities/Character.h"

namespace bitwave::entities {

class Player final : public Character {
public:
    Player();
    int score() const { return score_; }
    void addScore(int amount = 1) { score_ += amount; }
    void setSkinId(std::int64_t id) { skinId_ = id; }
    std::int64_t skinId() const { return skinId_; }
    void jump();
    void flap();

private:
    int score_{0};
    std::int64_t skinId_{0}; // 0 means the built-in skin.
};

} // namespace bitwave::entities
