#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <optional>

#include "shared/Contracts.h"

namespace bitwave::shared {

class GameAPI {
public:
    virtual ~GameAPI() = default;

    virtual LoginResult login(const std::string& username,
                               const std::string& password) = 0;

    virtual RegisterResult registerUser(const std::string& username,
                                         const std::string& password) = 0;

    virtual void logout() = 0;

    virtual std::optional<PlayerProfile> getProfile() = 0;
    virtual bool setAlias(const std::string& alias) = 0;
    virtual std::vector<LeaderboardEntry> getLeaderboard(std::size_t limit = 10) = 0;

    virtual void startMode(GameModeType mode, Difficulty difficulty = Difficulty::Medium) = 0;

    // Pass false when a movement key is released.
    virtual void handleAction(PlayerAction action, bool pressed = true) = 0;

    virtual void update(float deltaTime) = 0;

    virtual GameState getGameState() = 0;

    virtual long long getWalletBalance() = 0;

    virtual std::vector<StoreItem> getStoreItems() = 0;

    virtual PurchaseResult purchaseItem(std::int64_t itemId) = 0;
    virtual std::vector<CosmeticItem> getCosmetics() = 0;
    virtual bool equipCosmetic(std::int64_t itemId) = 0;
};

} // namespace bitwave::shared
