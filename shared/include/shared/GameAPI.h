#pragma once

// GameAPI — the ONLY boundary the frontend is allowed to call through.
//
//   Frontend --> GameAPI --> Backend Systems --> Persistence
//
// Never:
//   Frontend --> SQLite
//   Frontend --> internal backend classes (AuthService, Wallet, repositories, ...)
//
// This header documents the intended contract for Day 2+. It is a design
// contract at the repository-initialization stage, not a requirement that
// every function be implemented yet.

#include <string>
#include <vector>

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

    virtual PlayerProfile getProfile() = 0;

    virtual void startMode(GameModeType mode) = 0;

    virtual void handleAction(PlayerAction action) = 0;

    virtual void update(float deltaTime) = 0;

    virtual GameState getGameState() = 0;

    virtual long long getWalletBalance() = 0;

    virtual std::vector<StoreItem> getStoreItems() = 0;

    virtual PurchaseResult purchaseItem(int itemId) = 0;
};

} // namespace bitwave::shared
