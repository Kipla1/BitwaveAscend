#pragma once

#include <cstdint>
#include <optional>
#include "shared/Contracts.h"

namespace bitwave::player {

class PlayerProfileService {
public:
    virtual ~PlayerProfileService() = default;

    // Profile for the currently logged-in player. Empty if nobody is logged in.
    virtual std::optional<shared::PlayerProfile> getCurrentProfile() const = 0;

    // Called when a run ends. Only raises highest_score if `score` beats it.
    virtual void reportScore(long long score) = 0;

    virtual void setLevel(int level) = 0;
};

} // namespace bitwave::player