#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bitwave::persistence {

enum class OrderStatus { Pending, Succeeded, Failed };

struct PaymentOrder {
    std::int64_t id = 0;
    std::int64_t userId = 0;
    std::string  reference;
    long long    amountCents = 0;
    std::string  currency;
    long long    coins = 0;
    OrderStatus  status = OrderStatus::Pending;
};

class PaymentOrderRepository {
public:
    virtual ~PaymentOrderRepository() = default;

    // Throws if the reference already exists (it is UNIQUE).
    virtual void create(std::int64_t userId, const std::string& reference,
                        long long amountCents, const std::string& currency,
                        long long coins) = 0;

    virtual std::optional<PaymentOrder> findByReference(const std::string& reference) const = 0;

    // True only if the order was pending and has now moved to succeeded.
    // This is what stops a payment from being credited twice.
    virtual bool markSucceeded(const std::string& reference) = 0;

    // True only if the order was pending and has now moved to failed.
    virtual bool markFailed(const std::string& reference) = 0;
};

} // namespace bitwave::persistence