#pragma once

#include <unordered_map>
#include "persistence/UserRepository.h"

namespace bitwave::persistence {

// Fast in-memory fake for unit tests.
class InMemoryUserRepository : public UserRepository {
public:
    std::optional<UserRecord> findByUsername(const std::string& username) const override {
        auto it = users_.find(username);
        if (it == users_.end()) return std::nullopt;
        return it->second;
    }

    std::optional<UserRecord> findByGoogleSub(const std::string& googleSub) const override {
        if (googleSub.empty()) return std::nullopt;
        for (const auto& [username, record] : users_) {
            if (record.googleSub == googleSub) {
                return record;
            }
        }
        return std::nullopt;
    }

    std::optional<std::int64_t> create(const std::string& username,
                                       const std::string& passwordHash) override {
        if (users_.count(username) != 0 || !isAliasAvailable(username)) {
            return std::nullopt;
        }
        UserRecord record;
        record.id = nextId_++;
        record.username = username;
        record.passwordHash = passwordHash;
        record.alias = username; // default alias at registration; changeable via updateAlias
        users_.emplace(username, record);
        return record.id;
    }

    std::optional<std::int64_t> createGoogleUser(const std::string& username,
                                                 const std::string& email,
                                                 const std::string& googleSub) override {
        if (googleSub.empty() || users_.count(username) != 0 ||
            !isAliasAvailable(username) || findByGoogleSub(googleSub).has_value()) {
            return std::nullopt;
        }
        UserRecord record;
        record.id = nextId_++;
        record.username = username;
        record.passwordHash = ""; // Google-only account has no password hash
        record.alias = username;
        record.email = email;
        record.googleSub = googleSub;
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