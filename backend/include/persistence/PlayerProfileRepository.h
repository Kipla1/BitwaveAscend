#pragma once

#include <cstdint>
#include <optional>

namespace bitwave::persistence {

struct ProgressRecord {
    long long highestScore = 0;
    int       currentLevel = 0;
};

class PlayerProfileRepository {
public:
    virtual ~PlayerProfileRepository() = default;

    // Creates the row with default values. Called once, at registration.
    virtual void create(std::int64_t userId) = 0;

    virtual std::optional<ProgressRecord> find(std::int64_t userId) const = 0;

    virtual void updateHighestScore(std::int64_t userId, long long score) = 0;

    virtual void updateLevel(std::int64_t userId, int level) = 0;
};

} // namespace bitwave::persistence