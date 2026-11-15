#pragma once

#include <cstdint>
#include <optional>

#include "auth/AuthService.h"
#include "auth/GoogleAuthProvider.h"
#include "auth/PasswordHasher.h"
#include "auth/Session.h"
#include "persistence/PlayerProfileRepository.h"
#include "persistence/UserRepository.h"
#include "persistence/WalletRepository.h"

namespace bitwave::auth {

class AuthServiceImpl : public AuthService {
public:
    // All references must outlive this object.
    AuthServiceImpl(persistence::UserRepository& users,
                    persistence::PlayerProfileRepository& progress,
                    persistence::WalletRepository& wallets,
                    const PasswordHasher& hasher,
                    Session& session);

    AuthServiceImpl(persistence::UserRepository& users,
                    persistence::PlayerProfileRepository& progress,
                    persistence::WalletRepository& wallets,
                    const PasswordHasher& hasher,
                    Session& session,
                    GoogleAuthProvider& googleProvider);

    shared::RegisterResult registerUser(const std::string& username,
                                        const std::string& password) override;
    shared::LoginResult login(const std::string& username,
                              const std::string& password) override;
    shared::LoginResult loginWithGoogle() override;
    void logout() override;

private:
    persistence::UserRepository& users_;
    persistence::PlayerProfileRepository& progress_;
    persistence::WalletRepository& wallets_;
    const PasswordHasher& hasher_;
    Session& session_;
    GoogleAuthProvider* googleProvider_ = nullptr;
};

} // namespace bitwave::auth