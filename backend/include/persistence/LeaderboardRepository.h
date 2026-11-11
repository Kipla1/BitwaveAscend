#pragma once

#include <cstddef>
#include <vector>
#include "shared/Contracts.h"

namespace bitwave::persistence {

class LeaderboardRepository {
public:
    virtual ~LeaderboardRepository() = default;
    virtual std::vector<shared::LeaderboardEntry> top(std::size_t limit) const = 0;
};

} // namespace bitwave::persistence
