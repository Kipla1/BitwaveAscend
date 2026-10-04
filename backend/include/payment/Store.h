#pragma once

#include <optional>
#include <vector>
#include "shared/Contracts.h"

namespace bitwave::payment {

class Store {
public:
    virtual ~Store() = default;

    virtual std::vector<shared::StoreItem> listItems() const = 0;

    virtual std::optional<shared::StoreItem> getItem(std::int64_t itemId) const = 0;
};

} // namespace bitwave::payment