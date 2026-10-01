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
        record.alias = username; // default alias at registration; changeable via updateAlias
        users_.emplace(username, record);
        return record.id;
    }

    bool isAliasAvailable(const std::string& alias) const override {
        for (const auto& [username, record] : users_) {
            if (record.alias == alias) return false;
        }
        return true;
    }

    bool updateAlias(std::int64_t userId, const std::string& alias) override {
        if (!isAliasAvailable(alias)) return false; // single-threaded test fake; no race to guard against
        for (auto& [username, record] : users_) {
            if (record.id == userId) {
                record.alias = alias;
                return true;
            }
        }
        return false; // userId not found
    }

private:
    std::unordered_map<std::string, UserRecord> users_;
    std::int64_t nextId_ = 1;
};

} // namespace bitwave::persistence