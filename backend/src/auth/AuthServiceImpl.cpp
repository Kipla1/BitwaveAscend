#include "auth/AuthServiceImpl.h"

#include <algorithm>
#include <cctype>
#include <string>

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

// Derives a valid 3..15 character base username from an email local-part
// so there is always room for a "_1", "_2", ... suffix within 20 chars.
std::string deriveBaseUsername(const std::string& email) {
    const auto atPos = email.find('@');
    const std::string raw = (atPos == std::string::npos) ? email : email.substr(0, atPos);

    std::string cleaned;
    cleaned.reserve(raw.size());
    for (unsigned char c : raw) {
        if (std::isalnum(c) || c == '_') {
            cleaned.push_back(static_cast<char>(c));
        } else {
            cleaned.push_back('_');
        }
    }

    if (cleaned.size() < kMinUsernameLength) {
        cleaned = "player";
    }
    if (cleaned.size() > 15) {
        cleaned.resize(15);
    }
    return cleaned;
}

} // namespace

AuthServiceImpl::AuthServiceImpl(persistence::UserRepository& users,
                                 persistence::PlayerProfileRepository& progress,
                                 persistence::WalletRepository& wallets,
                                 const PasswordHasher& hasher,
                                 Session& session)
    : users_(users),
      progress_(progress),
      wallets_(wallets),
      hasher_(hasher),
      session_(session),
      googleProvider_(nullptr) {}

AuthServiceImpl::AuthServiceImpl(persistence::UserRepository& users,
                                 persistence::PlayerProfileRepository& progress,
                                 persistence::WalletRepository& wallets,
                                 const PasswordHasher& hasher,
                                 Session& session,
                                 GoogleAuthProvider& googleProvider)
    : users_(users),
      progress_(progress),
      wallets_(wallets),
      hasher_(hasher),
      session_(session),
      googleProvider_(&googleProvider) {}

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
    wallets_.create(*userId);

    result.success = true;
    result.message = "Account created.";
    return result;
}

shared::LoginResult AuthServiceImpl::login(const std::string& username,
                                           const std::string& password) {
    shared::LoginResult result;
    result.message = "Invalid username or password."; // same text for all failure cases

    const auto user = users_.findByUsername(username);
    // Google-only accounts have an empty passwordHash and must never be
    // allowed to log in via the password path.
    if (!user || user->passwordHash.empty() || !hasher_.verify(user->passwordHash, password)) {
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

shared::LoginResult AuthServiceImpl::loginWithGoogle() {
    shared::LoginResult result;
    if (!googleProvider_) {
        result.message = "Google sign-in is not configured.";
        return result;
    }

    const GoogleIdentity identity = googleProvider_->authenticate();
    if (!identity.ok || identity.googleSub.empty()) {
        result.message = identity.error.empty()
                             ? "Google sign-in failed."
                             : identity.error;
        return result;
    }

    // Returning Google player: log directly into their existing account.
    if (const auto existing = users_.findByGoogleSub(identity.googleSub)) {
        session_.start({existing->id, existing->username, existing->alias});
        result.success = true;
        result.message = "Logged in.";
        result.profile.userId = existing->id;
        result.profile.username = existing->username;
        result.profile.alias = existing->alias;
        return result;
    }

    // First-time Google player: provision an account with a unique username.
    const std::string base = deriveBaseUsername(identity.email);
    std::string chosenUsername = base;
    std::optional<std::int64_t> userId =
        users_.createGoogleUser(chosenUsername, identity.email, identity.googleSub);

    for (int suffix = 1; !userId && suffix <= 100; ++suffix) {
        chosenUsername = base + "_" + std::to_string(suffix);
        userId = users_.createGoogleUser(chosenUsername, identity.email, identity.googleSub);
    }

    if (!userId) {
        result.message = "Could not create account for Google user.";
        return result;
    }

    progress_.create(*userId);
    wallets_.create(*userId);

    session_.start({*userId, chosenUsername, chosenUsername});
    result.success = true;
    result.message = "Logged in.";
    result.profile.userId = *userId;
    result.profile.username = chosenUsername;
    result.profile.alias = chosenUsername;
    return result;
}

void AuthServiceImpl::logout() {
    session_.end();
}

} // namespace bitwave::auth