#include "core/GameAPIImpl.h"

#include <stdexcept>

namespace bitwave::core {

namespace {

[[noreturn]] void missingService(const char* methodName) {
    throw std::logic_error(
        std::string("GameAPIImpl::") + methodName +
        " needs a configured backend service.");
}

} // namespace

GameAPIImpl::GameAPIImpl(auth::AuthService& authService,
                         player::PlayerProfileService& profileService,
                         auth::AliasService* aliases,
                         persistence::LeaderboardRepository* leaderboard)
    : authService_(authService), profileService_(profileService), aliases_(aliases),
      leaderboard_(leaderboard) {}

GameAPIImpl::GameAPIImpl(auth::AuthService& authService,
                         player::PlayerProfileService& profileService,
                         GameManager& games, payment::Wallet& wallet,
                         payment::Store& store, payment::PaymentService& payments,
                         player::Inventory& inventory, auth::AliasService* aliases,
                         persistence::LeaderboardRepository* leaderboard)
    : GameAPIImpl(authService, profileService, aliases, leaderboard) {
    games_=&games; wallet_=&wallet; store_=&store;
    payments_=&payments; inventory_=&inventory;
}

shared::LoginResult GameAPIImpl::login(const std::string& username,
                                       const std::string& password) {
    auto result=authService_.login(username, password);
    if (result.success) {
        if (const auto profile=profileService_.getCurrentProfile()) {
            result.profile=*profile;
            equippedSkinId_=profile->equippedSkinId;
            if (store_ && inventory_ && !equipCosmetic(equippedSkinId_)) {
                equippedSkinId_=0;
                profileService_.setEquippedSkin(0);
                result.profile.equippedSkinId=0;
            }
            if (games_) games_->setSkinId(equippedSkinId_);
        }
    }
    return result;
}

shared::RegisterResult GameAPIImpl::registerUser(const std::string& username,
                                                 const std::string& password) {
    return authService_.registerUser(username, password);
}

void GameAPIImpl::logout() {
    equippedSkinId_=0;
    if (games_) games_->setSkinId(0);
    authService_.logout();
}

std::optional<shared::PlayerProfile> GameAPIImpl::getProfile() {
    return profileService_.getCurrentProfile();
}

bool GameAPIImpl::setAlias(const std::string& alias) {
    return aliases_ && aliases_->setAlias(alias);
}

std::vector<shared::LeaderboardEntry> GameAPIImpl::getLeaderboard(std::size_t limit) {
    if (!leaderboard_) missingService("getLeaderboard");
    return leaderboard_->top(limit);
}

void GameAPIImpl::startMode(shared::GameModeType mode, shared::Difficulty difficulty) {
    if (!games_) missingService("startMode");
    games_->startMode(mode, difficulty);
    games_->setSkinId(equippedSkinId_);
    savedRunScore_=0;
}

void GameAPIImpl::handleAction(shared::PlayerAction action, bool pressed) {
    if (!games_) missingService("handleAction");
    games_->handleAction(action, pressed);
}

void GameAPIImpl::update(float deltaTime) {
    if (!games_) missingService("update");
    games_->update(deltaTime);
    const auto state=games_->state();
    if (state.score > savedRunScore_) {
        profileService_.reportScore(state.score);
        savedRunScore_=state.score;
    }
}

shared::GameState GameAPIImpl::getGameState() {
    if (!games_) missingService("getGameState");
    return games_->state();
}

long long GameAPIImpl::getWalletBalance() {
    if (!wallet_) missingService("getWalletBalance");
    return wallet_->getBalance();
}

std::vector<shared::StoreItem> GameAPIImpl::getStoreItems() {
    if (!store_) missingService("getStoreItems");
    return store_->listItems();
}

shared::PurchaseResult GameAPIImpl::purchaseItem(std::int64_t itemId) {
    if (!payments_) missingService("purchaseItem");
    return payments_->purchaseItem(itemId);
}

std::vector<shared::CosmeticItem> GameAPIImpl::getCosmetics() {
    if (!store_ || !inventory_) missingService("getCosmetics");
    std::vector<shared::CosmeticItem> cosmetics;
    cosmetics.push_back({0, "Starter Skin", "skin.default", 0, true,
                         equippedSkinId_==0});
    const auto ownedItems=inventory_->listItems();
    for (const auto& item : store_->listItems()) {
        if (item.kind!=shared::StoreItemKind::Cosmetic) continue;
        bool owned=item.price==0;
        for (const auto& entry : ownedItems)
            if (entry.itemId==item.itemId && entry.quantity>0) owned=true;
        cosmetics.push_back({item.itemId,item.name,item.resourceKey,item.price,
                             owned,item.itemId==equippedSkinId_});
    }
    return cosmetics;
}

bool GameAPIImpl::equipCosmetic(std::int64_t itemId) {
    if (!store_ || !inventory_) missingService("equipCosmetic");
    if (itemId==0) {
        equippedSkinId_=0;
        profileService_.setEquippedSkin(0);
        if (games_) games_->setSkinId(0);
        return true;
    }
    const auto item=store_->getItem(itemId);
    if (!item || item->kind!=shared::StoreItemKind::Cosmetic) return false;
    bool owned=item->price==0;
    for (const auto& entry : inventory_->listItems())
        if (entry.itemId==itemId && entry.quantity>0) owned=true;
    if (!owned) return false;
    equippedSkinId_=itemId;
    profileService_.setEquippedSkin(itemId);
    if (games_) games_->setSkinId(itemId);
    return true;
}

} // namespace bitwave::core
