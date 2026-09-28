#pragma once

#include <unordered_map>
#include "persistence/UserRepository.h"

namespace bitwave::persistence {

// Temporary stand-in until the SQLite repository exists. Also useful
// permanently as a fast fake for unit tests.
class InMemoryUserRepository : public UserRepository {
public:
    std::optional<UserRecord> findByUsername(const std::string& username) const override {
        auto it = users_.find(username);
        if (it == users_.end()) return std::nullopt;
        return it->second;
    }

    std::optional<std::int64_t> create(const std::string& username,
                                       const std::string& passwordHash) override {
        if (users_.count(username) != 0) return std::nullopt;
        UserRecord record;
        record.id = nextId_++;
        record.username = username;
        record.passwordHash = passwordHash;
        record.alias = username; // placeholder until AliasService (Day 4)
        users_.emplace(username, record);
        return record.id;
    }

private:
    std::unordered_map<std::string, UserRecord> users_;
    std::int64_t nextId_ = 1;
};

} // namespace bitwave::persistence