#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bitwave::persistence {

struct UserRecord {
    std::int64_t id = 0;
    std::string  username;
    std::string  passwordHash;
    std::string  alias;
};

class UserRepository {
public:
    virtual ~UserRepository() = default;

    virtual std::optional<UserRecord> findByUsername(const std::string& username) const = 0;

    virtual std::optional<std::int64_t> create(const std::string& username,
                                               const std::string& passwordHash) = 0;

    virtual bool isAliasAvailable(const std::string& alias) const =0;

    virtual bool updateAlias(std::int64_t userId, const std::string& alias) = 0;
};

} // namespace bitwave::persistence