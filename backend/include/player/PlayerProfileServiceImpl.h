#pragma once

#include "auth/Session.h"
#include "persistence/PlayerProfileRepository.h"
#include "player/PlayerProfile.h"

namespace bitwave::player {

class PlayerProfileServiceImpl : public PlayerProfileService {
public:
    PlayerProfileServiceImpl(persistence::PlayerProfileRepository& progress,
                             const auth::Session& session);

    std::optional<shared::PlayerProfile> getCurrentProfile() const override;
    void reportScore(long long score) override;
    void setLevel(int level) override;

private:
    persistence::PlayerProfileRepository& progress_;
    const auth::Session& session_; // read-only: this service never logs anyone in or out
};

} // namespace bitwave::player