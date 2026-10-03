#pragma once

#include <optional>
#include <vector>
#include "shared/Contracts.h"

namespace bitwave::persistence {

class StoreItemRepository {
public:
    virtual ~StoreItemRepository() = default;

    virtual std::vector<shared::StoreItem> listAll() const = 0;

    virtual std::optional<shared::StoreItem> findById(std::int64_t itemId) const = 0;
};

} // namespace bitwave::persistence