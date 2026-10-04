#pragma once

#include <cstdint>
#include "shared/Contracts.h"

namespace bitwave::payment {

class PaymentService {
public:
    virtual ~PaymentService() = default;

    virtual shared::PurchaseResult purchaseItem(std::int64_t itemId) = 0;
};

} // namespace bitwave::payment