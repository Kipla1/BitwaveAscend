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

shared::PlayerProfile GameAPIImpl::getProfile() {
    // GameAPI's contract returns PlayerProfile by value, with no way to say
    // "nobody is logged in" — a default-constructed, empty profile is the
    // closest fit today. Worth raising with the team: should GameAPI
    // distinguish "not logged in" explicitly (e.g. via std::optional, or a
    // documented empty userId == 0 convention)?
    return profileService_.getCurrentProfile().value_or(shared::PlayerProfile{});
}

void GameAPIImpl::startMode(shared::GameModeType) { notImplemented("startMode"); }
void GameAPIImpl::handleAction(shared::PlayerAction) { notImplemented("handleAction"); }
void GameAPIImpl::update(float) { notImplemented("update"); }
shared::GameState GameAPIImpl::getGameState() { notImplemented("getGameState"); }
long long GameAPIImpl::getWalletBalance() { notImplemented("getWalletBalance"); }
std::vector<shared::StoreItem> GameAPIImpl::getStoreItems() { notImplemented("getStoreItems"); }
shared::PurchaseResult GameAPIImpl::purchaseItem(int) { notImplemented("purchaseItem"); }

} // namespace bitwave::core