#pragma once

#include <optional>

#include "auth/AliasService.h"
#include "auth/AuthService.h"
#include "core/GameManager.h"
#include "payment/PaymentService.h"
#include "payment/Store.h"
#include "payment/Wallet.h"
#include "player/Inventory.h"
#include "player/PlayerProfile.h"
#include "persistence/LeaderboardRepository.h"
#include "shared/GameAPI.h"

namespace bitwave::core {

// Frontend-facing adapter for accounts, gameplay, and player data.
class GameAPIImpl : public shared::GameAPI {
public:
    GameAPIImpl(auth::AuthService& authService, player::PlayerProfileService& profileService,
                auth::AliasService* aliases = nullptr,
                persistence::LeaderboardRepository* leaderboard = nullptr);
    GameAPIImpl(auth::AuthService& authService, player::PlayerProfileService& profileService,
                GameManager& games, payment::Wallet& wallet, payment::Store& store,
                payment::PaymentService& payments, player::Inventory& inventory,
                auth::AliasService* aliases = nullptr,
                persistence::LeaderboardRepository* leaderboard = nullptr);

    shared::LoginResult login(const std::string& username,
                              const std::string& password) override;
    shared::RegisterResult registerUser(const std::string& username,
                                        const std::string& password) override;
    void logout() override;
    std::optional<shared::PlayerProfile> getProfile() override;
    bool setAlias(const std::string& alias) override;
    std::vector<shared::LeaderboardEntry> getLeaderboard(std::size_t limit = 10) override;

    void startMode(shared::GameModeType mode,
                   shared::Difficulty difficulty = shared::Difficulty::Medium) override;
    void handleAction(shared::PlayerAction action, bool pressed = true) override;
    void update(float deltaTime) override;
    shared::GameState getGameState() override;
    long long getWalletBalance() override;
    std::vector<shared::StoreItem> getStoreItems() override;
    shared::PurchaseResult purchaseItem(std::int64_t itemId) override;
    std::vector<shared::CosmeticItem> getCosmetics() override;
    bool equipCosmetic(std::int64_t itemId) override;

private:
    auth::AuthService& authService_;
    player::PlayerProfileService& profileService_;
    GameManager* games_{nullptr};
    payment::Wallet* wallet_{nullptr};
    payment::Store* store_{nullptr};
    payment::PaymentService* payments_{nullptr};
    player::Inventory* inventory_{nullptr};
    auth::AliasService* aliases_{nullptr};
    persistence::LeaderboardRepository* leaderboard_{nullptr};
    std::int64_t equippedSkinId_{0};
    long long savedRunScore_{0};
};

} // namespace bitwave::core
