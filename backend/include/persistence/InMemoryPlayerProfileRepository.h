#pragma once

#include <unordered_map>
#include "persistence/PlayerProfileRepository.h"

namespace bitwave::persistence {

// Fast fake for tests. Mirrors InMemoryUserRepository's role.
class InMemoryPlayerProfileRepository : public PlayerProfileRepository {
public:
    void create(std::int64_t userId) override {
        progress_[userId] = ProgressRecord{};
    }

    std::optional<ProgressRecord> find(std::int64_t userId) const override {
        auto it = progress_.find(userId);
        if (it == progress_.end()) return std::nullopt;
        return it->second;
    }

    void updateHighestScore(std::int64_t userId, long long score) override {
        auto it = progress_.find(userId);
        if (it != progress_.end() && score > it->second.highestScore) {
            it->second.highestScore = score;
        }
    }

    void updateLevel(std::int64_t userId, int level) override {
        auto it = progress_.find(userId);
        if (it != progress_.end()) {
            it->second.currentLevel = level;
        }
    }

private:
    std::unordered_map<std::int64_t, ProgressRecord> progress_;
};

} // namespace bitwave::persistence