#pragma once

#include <string>
#include "shared/Contracts.h"

namespace bitwave::auth {

class AuthService {
public:
    virtual ~AuthService() = default;

    virtual bitwave::shared::RegisterResult registerUser(
        const std::string& username,
        const std::string& password) = 0;

    virtual bitwave::shared::LoginResult login(
        const std::string& username,
        const std::string& password) = 0;

    virtual void logout() = 0;
};

} // namespace bitwave::auth