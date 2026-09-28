#pragma once

#include <cstdint>
#include <optional>

#include "auth/AuthService.h"
#include "auth/PasswordHasher.h"
#include "persistence/UserRepository.h"

namespace bitwave::auth {

class AuthServiceImpl : public AuthService {
public:
    // Both references must outlive this object.
    AuthServiceImpl(persistence::UserRepository& users, const PasswordHasher& hasher);

    shared::RegisterResult registerUser(const std::string& username,
                                        const std::string& password) override;
    shared::LoginResult login(const std::string& username,
                              const std::string& password) override;
    void logout() override;

private:
    persistence::UserRepository& users_;
    const PasswordHasher& hasher_;
    std::optional<std::int64_t> currentUserId_; // stand-in until Session (Day 3+)
};

} // namespace bitwave::auth