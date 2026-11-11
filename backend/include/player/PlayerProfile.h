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

    // Saves a new personal best as soon as it is reached.
    virtual void reportScore(long long score) = 0;

    virtual void setLevel(int level) = 0;
    virtual void setEquippedSkin(std::int64_t itemId) = 0;
};

} // namespace bitwave::player
