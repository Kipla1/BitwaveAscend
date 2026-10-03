#pragma once

#include <optional>

#include "auth/AuthService.h"
#include "player/PlayerProfile.h"
#include "shared/GameAPI.h"

namespace bitwave::core {

// Thin pass-through from the frontend-facing contract to backend services.
// The auth/profile methods are real. Everything else throws until
// GameManager (Day 5+) and the wallet/store system (Day 10) exist —
// see the .cpp for why that's a deliberate choice, not a placeholder bug.
class GameAPIImpl : public shared::GameAPI {
public:
    GameAPIImpl(auth::AuthService& authService, player::PlayerProfileService& profileService);

    shared::LoginResult login(const std::string& username,
                              const std::string& password) override;
    shared::RegisterResult registerUser(const std::string& username,
                                        const std::string& password) override;
    void logout() override;
    std::optional<shared::PlayerProfile> getProfile() override;

    void startMode(shared::GameModeType mode) override;
    void handleAction(shared::PlayerAction action) override;
    void update(float deltaTime) override;
    shared::GameState getGameState() override;
    long long getWalletBalance() override;
    std::vector<shared::StoreItem> getStoreItems() override;
    shared::PurchaseResult purchaseItem(int itemId) override;

private:
    auth::AuthService& authService_;
    player::PlayerProfileService& profileService_;
};

} // namespace bitwave::core