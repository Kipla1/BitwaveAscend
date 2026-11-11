#include "player/PlayerProfileServiceImpl.h"

namespace bitwave::player {

PlayerProfileServiceImpl::PlayerProfileServiceImpl(
    persistence::PlayerProfileRepository& progress, const auth::Session& session)
    : progress_(progress), session_(session) {}

std::optional<shared::PlayerProfile> PlayerProfileServiceImpl::getCurrentProfile() const {
    if (!session_.isActive()) {
        return std::nullopt;
    }
    const auto& user = *session_.user();
    const auto record = progress_.find(user.id);
    if (!record) {
        return std::nullopt; // registered but progress row missing — shouldn't happen
    }

    shared::PlayerProfile profile;
    profile.userId = user.id;
    profile.username = user.username;
    profile.alias = user.alias;
    profile.highestScore = record->highestScore;
    profile.currentLevel = record->currentLevel;
    profile.equippedSkinId = record->equippedSkinId;
    return profile;
}

void PlayerProfileServiceImpl::reportScore(long long score) {
    if (!session_.isActive()) return;
    progress_.updateHighestScore(session_.user()->id, score);
}

void PlayerProfileServiceImpl::setLevel(int level) {
    if (!session_.isActive()) return;
    progress_.updateLevel(session_.user()->id, level);
}

void PlayerProfileServiceImpl::setEquippedSkin(std::int64_t itemId) {
    if (!session_.isActive()) return;
    progress_.updateEquippedSkin(session_.user()->id, itemId);
}

} // namespace bitwave::player
