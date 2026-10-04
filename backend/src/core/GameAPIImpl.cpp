#include "core/GameAPIImpl.h"

#include <stdexcept>

namespace bitwave::core {

namespace {

[[noreturn]] void notImplemented(const char* methodName) {
    throw std::logic_error(
        std::string("GameAPIImpl::") + methodName +
        " is not implemented yet (awaiting GameManager / Wallet).");
}

} // namespace

GameAPIImpl::GameAPIImpl(auth::AuthService& authService,
                         player::PlayerProfileService& profileService)
    : authService_(authService), profileService_(profileService) {}

shared::LoginResult GameAPIImpl::login(const std::string& username,
                                       const std::string& password) {
    return authService_.login(username, password);
}

shared::RegisterResult GameAPIImpl::registerUser(const std::string& username,
                                                 const std::string& password) {
    return authService_.registerUser(username, password);
}

void GameAPIImpl::logout() {
    authService_.logout();
}

std::optional<shared::PlayerProfile> GameAPIImpl::getProfile() {
    return profileService_.getCurrentProfile();
}

void GameAPIImpl::startMode(shared::GameModeType) { notImplemented("startMode"); }
void GameAPIImpl::handleAction(shared::PlayerAction) { notImplemented("handleAction"); }
void GameAPIImpl::update(float) { notImplemented("update"); }
shared::GameState GameAPIImpl::getGameState() { notImplemented("getGameState"); }
long long GameAPIImpl::getWalletBalance() { notImplemented("getWalletBalance"); }
std::vector<shared::StoreItem> GameAPIImpl::getStoreItems() { notImplemented("getStoreItems"); }
shared::PurchaseResult GameAPIImpl::purchaseItem(int) { notImplemented("purchaseItem"); }

} // namespace bitwave::core