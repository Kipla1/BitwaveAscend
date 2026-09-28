#pragma once

#include <string>
#include "shared/Contracts.h"

namespace bitwave::auth {

// Pure virtual interface — no data members, no implementation.
// Implemented for real on Day 3 (SQLite + libsodium). Until then this
// exists purely so other modules (GameManager, tests) can be written
// against a stable contract.
class AuthService {
public:
    virtual ~AuthService() = default; // always virtual destructor on a base class

    virtual bitwave::shared::RegisterResult registerUser(
        const std::string& username,
        const std::string& password) = 0;

    virtual bitwave::shared::LoginResult login(
        const std::string& username,
        const std::string& password) = 0;

    virtual void logout() = 0;
};

} // namespace bitwave::auth