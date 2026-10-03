#include "auth/AuthServiceImpl.h"

#include <algorithm>
#include <cctype>

namespace bitwave::auth {

namespace {

constexpr std::size_t kMinUsernameLength = 3;
constexpr std::size_t kMaxUsernameLength = 20;
constexpr std::size_t kMinPasswordLength = 8;

bool isValidUsername(const std::string& username) {
    if (username.size() < kMinUsernameLength || username.size() > kMaxUsernameLength) {
        return false;
    }
    return std::all_of(username.begin(), username.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '_';
    });
}

} // namespace

AuthServiceImpl::AuthServiceImpl(persistence::UserRepository& users,
                                persistence::PlayerProfileRepository& progress,
                                const PasswordHasher& hasher,
                                Session& session)
    : users_(users),
      progress_(progress),
      hasher_(hasher),
      session_(session) {}

shared::RegisterResult AuthServiceImpl::registerUser(const std::string& username,
                                                     const std::string& password) {
    shared::RegisterResult result;

    if (!isValidUsername(username)) {
        result.message = "Username must be 3-20 characters: letters, numbers, underscore.";
        return result;
    }
    if (password.size() < kMinPasswordLength) {
        result.message = "Password must be at least 8 characters.";
        return result;
    }

    const std::string hash = hasher_.hash(password);
    if (hash.empty()) {
        result.message = "Could not create account. Please try again.";
        return result;
    }
    const auto userId = users_.create(username, hash);
    if (!userId) {
        result.message = "That username is already taken.";
        return result;
    }
    // TODO: once Game/app-setup exists, move this call up a level so
    // AuthServiceImpl doesn't need to know about player_progress (or any
    // future per-user table, e.g. wallets).
    progress_.create(*userId);

    result.success = true;
    result.message = "Account created.";
    return result;
}

shared::LoginResult AuthServiceImpl::login(const std::string& username,
                                           const std::string& password) {
    shared::LoginResult result;
    result.message = "Invalid username or password."; // same text for both failure cases

    const auto user = users_.findByUsername(username);
    if (!user || !hasher_.verify(user->passwordHash, password)) {
        return result;
    }

    session_.start({user->id, user->username, user->alias});
    result.success = true;
    result.message = "Logged in.";
    result.profile.userId = user->id;
    result.profile.username = user->username;
    result.profile.alias = user->alias;
    return result;
}

void AuthServiceImpl::logout() {
    session_.end();
}

} // namespace bitwave::auth