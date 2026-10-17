#pragma once

#include <string>

namespace bitwave::payment {

struct ChargeInit {
    bool        ok = false;
    std::string authorizationUrl; // where the player pays (opened in a browser)
    std::string error;
};

enum class ChargeStatus { Pending, Success, Failed };

struct ChargeVerification {
    bool         ok = false;      // false = could not reach or understand the provider
    ChargeStatus status = ChargeStatus::Pending;
    long long    amountCents = 0;
    std::string  currency;
    std::string  error;
};

// The only thing the rest of the backend knows about Paystack. The real
// implementation (HTTP) comes later; tests use FakePaymentGateway.
class PaymentGateway {
public:
    virtual ~PaymentGateway() = default;

    virtual ChargeInit initializeCharge(const std::string& email,
                                        long long amountCents,
                                        const std::string& currency,
                                        const std::string& reference) = 0;

    virtual ChargeVerification verifyCharge(const std::string& reference) = 0;
};

} // namespace bitwave::payment