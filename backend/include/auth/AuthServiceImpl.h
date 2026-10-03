#pragma once

#include <cstdint>
#include <optional>

#include "auth/Session.h"
#include "auth/AuthService.h"
#include "auth/PasswordHasher.h"
#include "persistence/UserRepository.h"
#include "persistence/PlayerProfileRepository.h"

namespace bitwave::auth {

class AuthServiceImpl : public AuthService {
public:
    // Both references must outlive this object.
    AuthServiceImpl(persistence::UserRepository& users,
                    persistence::PlayerProfileRepository& progress,
                    const PasswordHasher& hasher,
                    Session& session);

    shared::RegisterResult registerUser(const std::string& username,
                                        const std::string& password) override;
    shared::LoginResult login(const std::string& username,
                              const std::string& password) override;
    void logout() override;

private:
    persistence::UserRepository& users_;
    persistence::PlayerProfileRepository& progress_;
    const PasswordHasher& hasher_;
    Session& session_;
};

} // namespace bitwave::auth